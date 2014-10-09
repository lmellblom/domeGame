/*
Copyright (c) 2014 Miroslav Andel IXEL AB
miroslav.andel@ixel.se
All rights reserved.
*/

#include "sgct.h"
#include "Webserver.h"
#include "UserData.h"
#include "Quad.h"

#include <iostream>

#include <btBulletDynamicsCommon.h>

#define MAX_WEB_USERS 256

sgct::Engine * gEngine;

//BULLET HELLO WORLD
void bulletTest()
{

	btBroadphaseInterface* broadphase = new btDbvtBroadphase();

	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

	dynamicsWorld->setGravity(btVector3(0, -10, 0));


	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);

	btCollisionShape* fallShape = new btSphereShape(1);


	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
	btRigidBody::btRigidBodyConstructionInfo
		groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	dynamicsWorld->addRigidBody(groundRigidBody);


	btDefaultMotionState* fallMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 0)));
	btScalar mass = 1;
	btVector3 fallInertia(0, 0, 0);
	fallShape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
	btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
	dynamicsWorld->addRigidBody(fallRigidBody);


	for (int i = 0; i < 300; i++) {
		dynamicsWorld->stepSimulation(1 / 60.f, 10);

		btTransform trans;
		fallRigidBody->getMotionState()->getWorldTransform(trans);

		std::cout << "sphere height: " << trans.getOrigin().getY() << std::endl;
	}

	dynamicsWorld->removeRigidBody(fallRigidBody);
	delete fallRigidBody->getMotionState();
	delete fallRigidBody;

	dynamicsWorld->removeRigidBody(groundRigidBody);
	delete groundRigidBody->getMotionState();
	delete groundRigidBody;


	delete fallShape;

	delete groundShape;


	delete dynamicsWorld;
	delete solver;
	delete collisionConfiguration;
	delete dispatcher;
	delete broadphase;
}

void myInitFun();
void myDrawFun();
void myPreSyncFun();
void myPostSyncFun();
void myEncodeFun();
void myDecodeFun();
void myCleanUpFun();
void keyCallback(int key, int action);

void renderAvatars();
void renderConnectionInfo();

Webserver webserver;
UserData webUsers[MAX_WEB_USERS];
std::vector<UserData> webUsers_copy;

tthread::mutex mWebMutex; //used for thread exclusive data access (prevent corruption)

sgct::SharedFloat curr_time(0.0f);
sgct::SharedVector<UserData> sharedUserData;

bool takeScreenShot = false;
glm::mat4 MVP;

//shader uniforms
GLint Matrix_Loc = -1;
GLint Color_Loc = -1;
GLint Avatar_Tex_Loc = -1;

size_t avatarTex;

Quad avatar;

// tar emot data från html-sidan
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
        }
    }
    
    // to set the color on the figur... is done only once when the page starts at the user.
    // the color is set in the webbrowser at the moment..
    if (sscanf(msg, "rgb %u %f %f %f\n", &id, &color[0], &color[1], &color[2]) == 4){
        color[0] /= 255.0f;
        color[1] /= 255.0f;
        color[2] /= 255.0f;
        webUsers[id].setColor(color[0], color[1], color[2]);
        //fprintf(stderr, "%s\n", "sätt färgen på figuren!!! "); // debugsyfte
    }

}

int main( int argc, char* argv[] )
{
	bulletTest();
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

    // testing bullet
    btCollisionShape* fallShape = new btSphereShape(1);

	// Main loop
	gEngine->render();

	// Clean up (de-allocate)
	delete gEngine;

	// Exit program
	exit( EXIT_SUCCESS );
	
}

void myInitFun()
{
    avatar.create(0.8f, 0.8f);
    
    // load textures
    sgct::TextureManager::instance()->setAnisotropicFilterSize(8.0f);
	sgct::TextureManager::instance()->setCompression(sgct::TextureManager::S3TC_DXT);
    sgct::TextureManager::instance()->loadTexure(avatarTex, "avatar", "avatar.png", true);

	sgct::ShaderManager::instance()->addShaderProgram( "avatar",
			"avatar.vert",
			"avatar.frag" );

	sgct::ShaderManager::instance()->bindShaderProgram( "avatar" );
 
	Matrix_Loc = sgct::ShaderManager::instance()->getShaderProgram( "avatar").getUniformLocation( "MVP" );
    Color_Loc = sgct::ShaderManager::instance()->getShaderProgram( "avatar").getUniformLocation( "FaceColor" );
    Avatar_Tex_Loc = sgct::ShaderManager::instance()->getShaderProgram( "avatar").getUniformLocation( "Tex" );
    
	sgct::ShaderManager::instance()->unBindShaderProgram();

}

void myDrawFun()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    MVP = gEngine->getActiveModelViewProjectionMatrix();
    
    renderAvatars();

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

// renderar den fina figuren som visas. 
void renderAvatars()
{
    //float speed = 50.0f;
    float radius = 7.4f;
    float time_visible = 5.0f;
    
    glm::mat4 trans_mat = glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -radius));
    glm::vec3 color;
    
    sgct::ShaderManager::instance()->bindShaderProgram( "avatar" );
    avatar.bind();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture( GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByHandle(avatarTex) );
    
    for(unsigned int i=1; i<MAX_WEB_USERS; i++)
        if( curr_time.getVal() - webUsers_copy[i].getTimeStamp() < time_visible )
        {
            glm::mat4 thetaRot = glm::rotate( glm::mat4(1.0f),
                                             glm::degrees( webUsers_copy[i].getTheta() ),
                                             glm::vec3(0.0f, -1.0f, 0.0f));
            
            glm::mat4 phiRot = glm::rotate( glm::mat4(1.0f),
                                           90.0f - glm::degrees( webUsers_copy[i].getPhi() ),
                                           glm::vec3(1.0f, 0.0f, 0.0f));
            
            glm::mat4 avatarMat = MVP * thetaRot * phiRot * trans_mat;
            
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