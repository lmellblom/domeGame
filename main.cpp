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
#include "Game.h"
#include "Render.h"

#include <iostream>

#define MAX_WEB_USERS 256
#define DOME_RADIUS 7.4f

sgct::Engine * gEngine;
tthread::mutex mWebMutex; //used for thread exclusive data access (prevent corruption)

Webserver webserver;
UserData webUsers[MAX_WEB_USERS];
std::vector<UserData> webUsers_copy;

Simulation sim;

bool takeScreenShot = false;

sgct::SharedFloat curr_time(0.0f);
sgct::SharedFloat last_time(0.0f);
sgct::SharedVector<UserData> sharedUserData;

float pingedTime[MAX_WEB_USERS];
glm::vec3 pingedPosition[MAX_WEB_USERS];

Render *rendermanager = new Render;

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
    }
	else if (sscanf(msg, "signal %u\n", &id) == 1){
        fprintf(stderr, "%s %u\n", "alive from user ", id );
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
	gEngine->setInitOGLFunction( rendermanager->myInitFun );
	gEngine->setDrawFunction(rendermanager->myDrawFun);
	gEngine->setPreSyncFunction( myPreSyncFun );
	gEngine->setPostSyncPreDrawFunction( myPostSyncFun );
	gEngine->setCleanUpFunction( rendermanager->myCleanUpFun );
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

// function to be used when a user sends a ping. 
void ping(unsigned int id) {
    fprintf(stderr, "%s %u\n", "ping from the user ", id); // debug

	pingedTime[id] = static_cast<float>(sgct::Engine::getTime());
	pingedPosition[id] = sim.GetPlayerDirectionNonQuaternion(id);

}