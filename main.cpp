/*
Copyright (c) 2014 Miroslav Andel IXEL AB
miroslav.andel@ixel.se
All rights reserved.
*/

#include "sgct.h"
#include "Webserver.h"
#include "UserData.h"
#include "Quad.h"
#include "Simulation.h"

#include <iostream>

#define MAX_WEB_USERS 256
#define DOME_RADIUS 7.4f

sgct::Engine * gEngine;


void myInitFun();
void myDrawFun();
void myPreSyncFun();
void myPostSyncFun();
void myEncodeFun();
void myDecodeFun();
void myCleanUpFun();
void keyCallback(int key, int action);

void renderAvatars();
void renderSkyBox();
void renderConnectionInfo();
void renderBalls();

void ping(unsigned int id); // ping from user

Webserver webserver;
UserData webUsers[MAX_WEB_USERS];
std::vector<UserData> webUsers_copy;

tthread::mutex mWebMutex; //used for thread exclusive data access (prevent corruption)

sgct::SharedFloat curr_time(0.0f);
sgct::SharedFloat last_time(0.0f);
sgct::SharedVector<UserData> sharedUserData;

Simulation sim;

bool takeScreenShot = false;
glm::mat4 MVP;

//shader uniforms
GLint Matrix_Loc = -1;
GLint Color_Loc = -1;
GLint Avatar_Tex_Loc = -1;

size_t avatarTex;

size_t myTextureHandle; // for skyBox
sgct_utils::SGCTBox * myBox = NULL; 
GLint Matrix_Loc_Box = -1;
GLint Tex_Loc_Box; 
GLint Time_Loc;
GLint Curr_Time;
GLint Pos_Loc;
GLint Pings_Id;

float pingedTime[MAX_WEB_USERS];
glm::vec3 pingedPosition[MAX_WEB_USERS];
int pingedIds[MAX_WEB_USERS];
Quad avatar;
Quad ball;

// fetch data from the html site. do different things depending on the input
void webDecoder(const char * msg, size_t len)
{
    
    unsigned int id = 0;
    int posX = 0;
    int posY = 0;
    int colorPos = 0;
    float color[3];

    // fprintf(stderr, "Message from webpage: %s\n", msg); // debug syfte iaf man vill veta vad som fås i meddelandet
    
    if ( sscanf( msg, "pos %u %d %d\n", &id, &posX, &posY) == 3 )
    {
        if( id > 0 && id < MAX_WEB_USERS)
        {   
            mWebMutex.lock();
            webUsers[id].setCartesian2d(posX, posY, static_cast<float>(sgct::Engine::getTime()));
            mWebMutex.unlock();

			float s = webUsers[id].getS();
			float t = -webUsers[id].getT();
			float h = sqrt(1 - s*s - t*t);
			btVector3 pos(webUsers[id].getS(), h, -webUsers[id].getT());
			pos.normalize();
			pos *= 7.4;
			//calculate position vector
			sim.SetPlayerTarget(id, pos);
        }
    }
    
    // to set the color on the figur... is done only once when the page starts at the user.
    // the color is set in the webbrowser at the moment..
    else if (sscanf(msg, "rgb %u %f %f %f\n", &id, &color[0], &color[1], &color[2]) == 4){
        color[0] /= 255.0f;
        color[1] /= 255.0f;
        color[2] /= 255.0f;
        webUsers[id].setColor(color[0], color[1], color[2]);
        //fprintf(stderr, "%s\n", "sätt färgen på figuren!!! "); // debugsyfte
    }

	else if (sscanf(msg, "signal %u\n", &id) == 1){
        fprintf(stderr, "%s %u\n", "alive from user ", id );
		webUsers[id].setTimeStamp(static_cast<float>(sgct::Engine::getTime()));
	}

    else if (sscanf(msg, "ping %u\n", &id) == 1){
        ping(id);
    }

}

int main( int argc, char* argv[] )
{
	// Allocate
	gEngine = new sgct::Engine( argc, argv );

	// Bind your functions
	gEngine->setInitOGLFunction( myInitFun );
	gEngine->setDrawFunction( myDrawFun );
	gEngine->setPreSyncFunction( myPreSyncFun );
	gEngine->setPostSyncPreDrawFunction( myPostSyncFun );
	gEngine->setCleanUpFunction( myCleanUpFun );
    gEngine->setKeyboardCallbackFunction( keyCallback );
	sgct::SharedData::instance()->setEncodeFunction(myEncodeFun);
	sgct::SharedData::instance()->setDecodeFunction(myDecodeFun);

	// Init the engine
	if( !gEngine->init( sgct::Engine::OpenGL_3_3_Core_Profile ) )
	{
		delete gEngine;
		return EXIT_FAILURE;
	}

	webUsers_copy.assign(webUsers, webUsers + MAX_WEB_USERS);
    if( gEngine->isMaster() )
    {
        webserver.setCallback(webDecoder);
        //webserver.start(9000);
        webserver.start(80);
    }

	// Main loop
	gEngine->render();

	// Clean up (de-allocate)
	delete gEngine;

	// Exit program
	exit( EXIT_SUCCESS );
	
}

void myInitFun()
{
    avatar.create(0.8f, 0.8f); // how big
	ball.create(2.0f, 2.0f);

	for (int i = 0; i < MAX_WEB_USERS; i++) {
		pingedTime[i] = 0.0f;
		pingedPosition[i] = glm::vec3(0.f, 0.f, 0.f);
		pingedIds[i] = 0;
	}
	
    // load textures
    sgct::TextureManager::instance()->setAnisotropicFilterSize(8.0f);
	sgct::TextureManager::instance()->setCompression(sgct::TextureManager::S3TC_DXT);
    sgct::TextureManager::instance()->loadTexure(avatarTex, "avatar", "avatar.png", true);

    // add shaders
	sgct::ShaderManager::instance()->addShaderProgram( "avatar",
			"avatar.vert",
			"avatar.frag" );
	sgct::ShaderManager::instance()->bindShaderProgram( "avatar" );
 
	Matrix_Loc = sgct::ShaderManager::instance()->getShaderProgram( "avatar").getUniformLocation( "MVP" );
    Color_Loc = sgct::ShaderManager::instance()->getShaderProgram( "avatar").getUniformLocation( "FaceColor" );
    Avatar_Tex_Loc = sgct::ShaderManager::instance()->getShaderProgram( "avatar").getUniformLocation( "Tex" );

    // for the skyBox
    sgct::TextureManager::instance()->loadTexure(myTextureHandle, "skyBox", "sky.png", true);

    // add the box
    myBox = new sgct_utils::SGCTBox(2.0f, sgct_utils::SGCTBox::SkyBox);

    //Set up backface culling
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); //our polygon winding is counter clockwise
 
    sgct::ShaderManager::instance()->addShaderProgram( "xform",
            "PingShader.vert",
            "PingShader.frag" );
 
    sgct::ShaderManager::instance()->bindShaderProgram( "xform" );
 
    Matrix_Loc_Box = sgct::ShaderManager::instance()->getShaderProgram( "xform").getUniformLocation( "MVP" );
    Tex_Loc_Box = sgct::ShaderManager::instance()->getShaderProgram( "xform").getUniformLocation( "Tex" );
	Time_Loc = sgct::ShaderManager::instance()->getShaderProgram("xform").getUniformLocation("PingTime"); 
	Pos_Loc = sgct::ShaderManager::instance()->getShaderProgram("xform").getUniformLocation("PingPos");
	Curr_Time = sgct::ShaderManager::instance()->getShaderProgram("xform").getUniformLocation("CurrTime");
	Pings_Id = sgct::ShaderManager::instance()->getShaderProgram("xform").getUniformLocation("PingId");

    sgct::ShaderManager::instance()->unBindShaderProgram();
}

void myDrawFun()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    MVP = gEngine->getActiveModelViewProjectionMatrix();

    renderSkyBox();
	renderAvatars();
	renderBalls();

    //unbind shader program
    sgct::ShaderManager::instance()->unBindShaderProgram();

    glDisable(GL_BLEND);

}

void myPreSyncFun()
{
	//set the time only on the master
	if( gEngine->isMaster() )
	{
		//get the time in seconds
		curr_time.setVal( static_cast<float>(sgct::Engine::getTime()) );
        
        //copy webusers to rendering copy
        mWebMutex.lock();
        webUsers_copy.assign(webUsers, webUsers + MAX_WEB_USERS);
		mWebMutex.unlock();
		
        //Set the data that will be synced to the clients this frame
		sharedUserData.setVal(webUsers_copy);
	}
}

void myPostSyncFun()
{
	const float DISCONNECT_TIME = 5.0f;
	if (!gEngine->isMaster())
	{
		webUsers_copy = sharedUserData.getVal();
	}
    else
    {
        if(takeScreenShot)
        {
            gEngine->takeScreenshot();
            takeScreenShot = false;
        }
    }

	sim.Step(curr_time.getVal() - last_time.getVal());
	last_time.setVal(curr_time.getVal());
	for (unsigned int i = 1; i<MAX_WEB_USERS; i++)
		if (curr_time.getVal() - webUsers_copy[i].getTimeStamp() > DISCONNECT_TIME)
		{
			sim.RemovePlayer(i);
		}
}

void myEncodeFun()
{
	sgct::SharedData::instance()->writeFloat( &curr_time );
    sgct::SharedData::instance()->writeVector(&sharedUserData);
}

void myDecodeFun()
{
	sgct::SharedData::instance()->readFloat( &curr_time );
    sgct::SharedData::instance()->readVector(&sharedUserData);
}

void myCleanUpFun()
{
	avatar.clear();

    if(myBox != NULL)
        delete myBox;

}

void keyCallback(int key, int action)
{
	if( gEngine->isMaster() )
	{
		switch( key )
		{
            case SGCT_KEY_P:
            case SGCT_KEY_F10:
                if(action == SGCT_PRESS)
                    takeScreenShot = true;
                break;
        }
	}
}

void renderSkyBox()
{
    // för drawbox
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
  
    //create scene transform (animation)
    glm::mat4 scene_mat = glm::translate( glm::mat4(1.0f), glm::vec3( 0.0f, 0.0f, 0.0f) );

    glm::mat4 BoxMVP = MVP * scene_mat; // MVP = gEngine->getActiveModelViewProjectionMatrix()
 

    sgct::ShaderManager::instance()->bindShaderProgram( "xform" );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByHandle(myTextureHandle) );
  
    glUniformMatrix4fv(Matrix_Loc_Box, 1, GL_FALSE, &BoxMVP[0][0]);
    glUniform1i( Tex_Loc_Box, 0 );

	//**************** PING! *******************
		GLfloat time = static_cast<float>(sgct::Engine::getTime());
	// std::cout << "time: " << time << std::endl;
	glUniform3fv(Pos_Loc, MAX_WEB_USERS, &pingedPosition[0][0]);
	glUniform1fv(Time_Loc, MAX_WEB_USERS, &pingedTime[0]);
	glUniform1iv(Pings_Id, MAX_WEB_USERS, &pingedIds[0]);
	glUniform1f(Curr_Time, time);
	//**************** END PING *****************/

    //draw the box (to make the texture on inside)
	glFrontFace(GL_CW);
    myBox->draw();
	glFrontFace(GL_CCW);

    sgct::ShaderManager::instance()->unBindShaderProgram();
 
    glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );

}

// renderar den fina figuren som visas. 
void renderAvatars()
{
    float radius = 7.4f; //Domens radie
    
    glm::mat4 trans_mat = glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -radius));
    glm::vec3 color;
    
    sgct::ShaderManager::instance()->bindShaderProgram( "avatar" );
    avatar.bind();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByHandle(avatarTex) );
    
	//should really look over the rendering, maybe use more threads??
    for(unsigned int i=1; i<MAX_WEB_USERS; i++)
        if( sim.PlayerExists(i) )
		{	
			btQuaternion quat = sim.GetPlayerDirection(i);
			btVector3 axis = quat.getAxis();
			float angle = quat.getAngle();

			glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f),
				glm::degrees(angle),
				glm::vec3(axis.getX(), axis.getY(), axis.getZ()));

			glm::mat4 avatarMat = MVP * rot_mat * trans_mat;

            color.r = webUsers_copy[i].getRed();
            color.g = webUsers_copy[i].getGreen();
            color.b = webUsers_copy[i].getBlue();
            glUniformMatrix4fv(Matrix_Loc, 1, GL_FALSE, &avatarMat[0][0]);
            glUniform3f(Color_Loc, color.r, color.g, color.b);
            glUniform1i( Avatar_Tex_Loc, 0 );

            avatar.draw();
        }
    
	avatar.unbind();
}

void renderBalls() {
	float radius = 7.4f; //Domens radie

	glm::mat4 trans_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -radius));
	glm::vec3 color;

	sgct::ShaderManager::instance()->bindShaderProgram("avatar");
	ball.bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByHandle(avatarTex));

	//should really look over the rendering
	btQuaternion quat = sim.GetBallDirection(0);
	btVector3 axis = quat.getAxis();
	float angle = quat.getAngle();

	glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f),
		glm::degrees(angle),
		glm::vec3(axis.getX(), axis.getY(), axis.getZ()));

	glm::mat4 avatarMat = MVP * rot_mat * trans_mat;

	glUniformMatrix4fv(Matrix_Loc, 1, GL_FALSE, &avatarMat[0][0]);
	glUniform3f(Color_Loc, 1.0, 1.0, 1.0);
	glUniform1i(Avatar_Tex_Loc, 0);

	ball.draw();

	ball.unbind();
}

void renderPings() {
	float radius = 7.4f; //Domens radie

	glm::mat4 trans_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -radius));
	glm::vec3 color;

	sgct::ShaderManager::instance()->bindShaderProgram("avatar");
	ball.bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByHandle(avatarTex));

	//should really look over the rendering
	btQuaternion quat = sim.GetBallDirection(0);
	btVector3 axis = quat.getAxis();
	float angle = quat.getAngle();

	glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f),
		glm::degrees(angle),
		glm::vec3(axis.getX(), axis.getY(), axis.getZ()));

	glm::mat4 avatarMat = MVP * rot_mat * trans_mat;

	glUniformMatrix4fv(Matrix_Loc, 1, GL_FALSE, &avatarMat[0][0]);
	glUniform3f(Color_Loc, 1.0, 1.0, 1.0);
	glUniform1i(Avatar_Tex_Loc, 0);

	ball.draw();

	ball.unbind();
}

// function to be used when a user sends a ping. 
void ping(unsigned int id) {
    //fprintf(stderr, "%s %u\n", "ping from the user ", id); // debug

	pingedTime[id] = static_cast<float>(sgct::Engine::getTime());
	pingedPosition[id] = sim.GetPlayerDirectionNonQuaternion(id);
	
	int i = 0;
	while (pingedIds[i] != 0) {
		if (pingedIds[i] == id) {
			break;
		}
		
		i++;
	}
	pingedIds[i] = id;

	/*
	i = 0;
	while (pingedIds[i] != -1) {
		std::cout << "pingedIds[" << i << "] = " << pingedIds[i] << std::endl;
		i++;
	} */

}

//Vektor med alla users, vektor med tiden för alla users, globala tiden
