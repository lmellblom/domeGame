#include "sgct.h"
#include "Webserver.h"
#include "UserData.h"
#include "Quad.h"
#include "Simulation.h"
#include "Game.h"

#include <iostream>

#define MAX_WEB_USERS 20
#define DOME_RADIUS 7.4f

sgct::Engine * gEngine;

// for SGCT
void myInitFun();
void myDrawFun();
void myPreSyncFun();
void myPostSyncFun();
void myEncodeFun();
void myDecodeFun();
void myCleanUpFun();
void keyCallback(int key, int action);

Simulation sim;

//shared variables
sgct::SharedFloat curr_time(0.0f);
sgct::SharedFloat last_time(0.0f);

sgct::SharedVector<UserData> sharedUserData;
sgct::SharedObject<btQuaternion> sharedBallPos; // btQuartnions.. 
sgct::SharedObject<btQuaternion> sharedGoalPos;
sgct::SharedObject<int> sharedGameTeam; 

bool takeScreenShot = false;
glm::mat4 MVP;

//shader uniforms
GLint Matrix_Loc = -1;
GLint Color_Loc = -1;
GLint Avatar_Tex_Loc = -1;

size_t avatarTex;

size_t footballTex;
GLint Tex_Loc_Football;
GLint Matrix_Loc_Football;

size_t textureSkyBox; // for skyBox
sgct_utils::SGCTBox * myBox = NULL; 
GLint Matrix_Loc_Box = -1;
GLint Tex_Loc_Box; 
GLint Time_Loc;
GLint Curr_Time;
GLint Pos_Loc;
GLint Pings_Id;
GLint Ping_Col;
GLint Team_Loc;
GLint Matrix_Loc_Goal;
GLint Team_Loc_Goal;
GLint Goal_Loc_Tex = -1;

//float pingedTime[MAX_WEB_USERS];				lagt enskild i userdata istället!
//glm::vec3 pingedPosition[MAX_WEB_USERS];		lagt enskild i userdata istället!	

Quad avatar;
Quad goal;
Quad football;

Simulation sim;
Game game;


// fetch data from the html site. do different things depending on the input
void webDecoder(const char * msg, size_t len)
{
    unsigned int id = 0;
    int posX = 0;
    int posY = 0;
    int colorPos = 0;
    float color[3];

    //fprintf(stderr, "Message sent: %s\n", msg); // debug syfte iaf man vill veta vad som fås i meddelandet
    if ( sscanf( msg, "pos %u %d %d\n", &id, &posX, &posY) == 3 )
    {
        if( id > 0 && id < MAX_WEB_USERS)
        {   
            mWebMutex.lock();
            webUsers[id].setCartesian2d(posX, posY, static_cast<float>(sgct::Engine::getTime()));
            webUsers[id].setTeam(id); // sets the team depending on the id
            mWebMutex.unlock();

            btVector3 pos = webUsers[id].calculatePosition(); 
			//calculate position vector
			sim.SetPlayerTarget(id, pos); // creates a new player if it doesnt exist, else set target position
			webUsers[id].exists = true;

        }
    }
    
    // to set the color on the figur... is done only once when the page starts at the user.
    // the color is set in the webbrowser at the moment..
    else if (sscanf(msg, "rgb %u %f %f %f\n", &id, &color[0], &color[1], &color[2]) == 4){
        color[0] /= 255.0f;
        color[1] /= 255.0f;
        color[2] /= 255.0f;
        
        webUsers[id].setColor(color[0], color[1], color[2]);
    }
	else if (sscanf(msg, "signal %u\n", &id) == 1){
        //fprintf(stderr, "%s %u\n", "alive from user ", id );
		webUsers[id].setTimeStamp(static_cast<float>(sgct::Engine::getTime()));
	}

    else if (sscanf(msg, "ping %u\n", &id) == 1){
        ping(id);
    }

    else {
        //fprintf(stderr, "Message from webpage: %s\n", msg); // debug syfte iaf man vill veta vad som fås i meddelandet
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

    if( gEngine->isMaster() )
    {

        webserver.setCallback(webDecoder);
        //webserver.start(9000);
        webserver.start(80);

        //start server here
		//start sim here?
		//start game here?

    }

	// Main loop
	gEngine->render();

	// Clean up (de-allocate)
	delete gEngine;

	// Exit program
	exit( EXIT_SUCCESS );
	
}

void myInitFun() {

	sim.AddBall(0); // add the ball in the simulation

	// creates the quads
	avatar.create(2.4f, 2.4f); // how big
	football.create(1.5f, 1.5f); // a football instead of a white ball ;) 
	goal.create(2.0f, 2.0f);

	game.setGoal(); // set a random goal at the start


}

void myDrawFun()
{

}


void myPreSyncFun()
{	
	if (gEngine->isMaster())
	{
		
		//get the time in seconds
		curr_time.setVal( static_cast<float>(sgct::Engine::getTime()) );

		// simulation on master
		simulateMaster(); 

		// simulate the balls position. 
		sharedBallPos.setVal(sim.GetBallDirection(0)); 

		// set the goal target
		sharedGoalPos.setVal(game.getGoalQuaternion());
        
        //copy webusers to rendering copy
        mWebMutex.lock();
        webUsers_copy.assign(webUsers, webUsers + MAX_WEB_USERS);
		mWebMutex.unlock();
		
        //Set the data that will be synced to the clients this frame
		sharedUserData.setVal(webUsers_copy);

		// se if goal
		game.update(sim.GetBallDirectionNonQuaternion(0));
		sharedGameTeam.setVal(game.getTeam()); // kanske inte bästa idéen.
	}
}

void simulateMaster() {
	const float DISCONNECT_TIME = 5.0f;

	sim.Step(curr_time.getVal() - last_time.getVal());
	last_time.setVal(curr_time.getVal());

	for (unsigned int i = 1; i<MAX_WEB_USERS; i++) { // inte bra grej egentligen.. 
		if(webUsers[i].exists){ // only calculate on the users that are active. 
		    btVector3 pos = webUsers[i].calculatePosition(); 
			//calculate position vector
			sim.SetPlayerTarget(i, pos);

			btQuaternion direction = sim.GetPlayerDirection(i); 
			webUsers[i].setPlayerDirection(direction);

			if (curr_time.getVal() - webUsers[i].getTimeStamp() > DISCONNECT_TIME)
			{
				sim.RemovePlayer(i);
				webUsers[i].exists = false; 
			}
		}
	}

}

		curr_time.setVal(static_cast<float>(sgct::Engine::getTime()));
		sim.Step(curr_time.getVal() - last_time.getVal());
		last_time.setVal(curr_time.getVal());
		glm::vec3 v = sim.GetBallDirVec();
		std::cout << curr_time.getVal() << std::endl;
		//std::cout << v.x << " " << v.y << " " << v.z << std::endl;
	}
}



void myPostSyncFun()
{

}


void myEncodeFun()
{
	sgct::SharedData::instance()->writeFloat( &curr_time );
    sgct::SharedData::instance()->writeVector(&sharedUserData);
    sgct::SharedData::instance()->writeObj(&sharedBallPos);
    sgct::SharedData::instance()->writeObj(&sharedGoalPos);
    sgct::SharedData::instance()->writeObj(&sharedGameTeam);


}

void myDecodeFun()
{
	sgct::SharedData::instance()->readFloat( &curr_time );
    sgct::SharedData::instance()->readVector(&sharedUserData);
    sgct::SharedData::instance()->readObj(&sharedBallPos);
    sgct::SharedData::instance()->readObj(&sharedGoalPos);
    sgct::SharedData::instance()->readObj(&sharedGameTeam);

}

void myCleanUpFun()
{

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


// --------------------------------------------------------

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
    glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByHandle(textureSkyBox) );
  
    glUniformMatrix4fv(Matrix_Loc_Box, 1, GL_FALSE, &BoxMVP[0][0]);
    glUniform1i( Tex_Loc_Box, 0 );

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
    float radius = DOME_RADIUS; //Domens radie
    
    glm::mat4 trans_mat = glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -radius));
    glm::vec3 color;
    
    sgct::ShaderManager::instance()->bindShaderProgram( "avatar" );
    avatar.bind();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByHandle(avatarTex) );
    
	//should really look over the rendering, maybe use more threads??
    for(unsigned int i=1; i<MAX_WEB_USERS; i++)
        if( webUsers_copy[i].exists) //sim.PlayerExists(i) ) // does it exist
		{	
			btQuaternion quat = webUsers_copy[i].getPlayerDirection();//sim.GetPlayerDirection(i);
			btVector3 axis = quat.getAxis();
			float angle = quat.getAngle();
			float pingTime = webUsers_copy[i].getPingTime(); //pingedTime[i];
			float currTime = curr_time.getVal();
			int team = webUsers_copy[i].getTeam();

			glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f),
				glm::degrees(angle),
				glm::vec3(axis.getX(), axis.getY(), axis.getZ()));

			glm::mat4 avatarMat = MVP * rot_mat * trans_mat;

            color.r = webUsers_copy[i].getRed();
            color.g = webUsers_copy[i].getGreen();
            color.b = webUsers_copy[i].getBlue();
            glUniformMatrix4fv(Matrix_Loc, 1, GL_FALSE, &avatarMat[0][0]);
            glUniform3f(Color_Loc, color.r, color.g, color.b);
			glUniform1f(Time_Loc, pingTime);
			glUniform1f(Curr_Time, currTime);
			glUniform1i(Team_Loc, team);
            glUniform1i( Avatar_Tex_Loc, 0 );

            avatar.draw();
        }
    
	avatar.unbind();
}


void renderFootball() {
	float radius = DOME_RADIUS;
	glm::mat4 trans_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -radius));
	sgct::ShaderManager::instance()->bindShaderProgram("fotball");
	
	football.bind();
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByHandle(footballTex));
	
	btQuaternion quat = sharedBallPos.getVal();//sim.GetBallDirection(0);
	btVector3 axis = quat.getAxis();
	float angle = quat.getAngle();
	
	glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f),
	glm::degrees(angle),
	glm::vec3(axis.getX(), axis.getY(), axis.getZ()));

	glm::mat4 footballMat = MVP * rot_mat * trans_mat;
	glUniformMatrix4fv(Matrix_Loc_Football, 1, GL_FALSE, &footballMat[0][0]);
	glUniform1i(Tex_Loc_Football, 0);
	
	football.draw();
	
	football.unbind();

}

void renderGoal() {
	float radius = DOME_RADIUS;
	glm::mat4 trans_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -radius));
	sgct::ShaderManager::instance()->bindShaderProgram("goals");

	goal.bind();

	btQuaternion quat = sharedGoalPos.getVal();//game.getGoalQuaternion();
	btVector3 axis = quat.getAxis();
	float angle = quat.getAngle();
	int team = sharedGameTeam.getVal(); // bara så länge

	glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f),
		glm::degrees(angle),
		glm::vec3(axis.getX(), axis.getY(), axis.getZ()));

	glm::mat4 goalMat = MVP * rot_mat * trans_mat;
	glUniformMatrix4fv(Matrix_Loc_Goal, 1, GL_FALSE, &goalMat[0][0]);
	glUniform1i(Team_Loc_Goal, team);
	glUniform1i(Goal_Loc_Tex, 0);

	goal.draw();
	goal.unbind();

}

// function to be used when a user sends a ping. 
void ping(unsigned int id) {
    fprintf(stderr, "%s %u\n", "ping from the user ", id); // debug

    // only sets on master so no need to use the webCopy, also can use sim. 
    webUsers[id].setPingTime(static_cast<float>(sgct::Engine::getTime()));
    webUsers[id].setPingPosition(sim.GetPlayerDirectionNonQuaternion(id));
}
