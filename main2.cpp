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

}

void myDrawFun()
{

}


void myPreSyncFun()
{	
	if (gEngine->isMaster())
	{
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
	sgct::SharedData::instance()->writeFloat(&curr_time);
	sgct::SharedData::instance()->writeFloat(&last_time);
}

void myDecodeFun()
{
	sgct::SharedData::instance()->readFloat(&curr_time);
	sgct::SharedData::instance()->readFloat(&last_time);
}

void myCleanUpFun()
{

}

void keyCallback(int key, int action)
{

}