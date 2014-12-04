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

#include <iostream>

#define MAX_WEB_USERS 256
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

// rendering functions
void loadTexturesAndStuff();
void renderAvatars();
void renderSkyBox();
void renderConnectionInfo();
void renderFootball();
void renderGoal();

// ping
void ping(unsigned int id); // ping from user

// for the webserver and userdata
Webserver webserver;
UserData webUsers[MAX_WEB_USERS];
std::vector<UserData> webUsers_copy;

tthread::mutex mWebMutex; //used for thread exclusive data access (prevent corruption)

// shared variables
sgct::SharedFloat curr_time(0.0f);
sgct::SharedFloat last_time(0.0f);
sgct::SharedFloat goal_time(-100.0f);
sgct::SharedVector<UserData> sharedUserData;
sgct::SharedObject<btQuaternion> sharedBallPos; // btQuartnions.. 
sgct::SharedObject<btQuaternion> sharedGoalPos;

sgct::SharedObject<glm::vec3> sharedBallVec;
sgct::SharedObject<glm::vec3> sharedGoalVec;
sgct::SharedObject<glm::vec3> sharedPrevGoalVec;

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
GLint Tex_Loc_Box = -1;
GLint Time_Loc = -1;
GLint Curr_Time = -1;
GLint Pos_Loc = -1;
GLint Pings_Id = -1;
GLint Ping_Col = -1;
GLint Team_Loc = -1;
GLint Matrix_Loc_Goal = -1;
GLint Team_Loc_Goal = -1;
GLint Goal_Loc_Tex = -1;
GLint Goal_Vec_Loc = -1;
GLint Ball_Vec_Loc = -1;
GLint Prev_Goal_Vec_Loc = -1;
GLint CurrTime_Loc = -1;
GLint GoalTime_Loc = -1;

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
	if (sscanf(msg, "pos %u %d %d\n", &id, &posX, &posY) == 3)
	{
		if (id > 0 && id < MAX_WEB_USERS)
		{
			mWebMutex.lock();
			webUsers[id].setCartesian2d(posX, posY, static_cast<float>(sgct::Engine::getTime()));
			webUsers[id].setTeam(id); // sets the team depending on the id

			btVector3 pos = webUsers[id].calculatePosition();
			//calculate position vector
			sim.SetPlayerTarget(id, pos); // creates a new player if it doesnt exist, else set target position
			webUsers[id].exists = true;
			mWebMutex.unlock();


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
		//fprintf(stderr, "%s %u\n", "alive from user ", id);
		webUsers[id].setTimeStamp(static_cast<float>(sgct::Engine::getTime()));
	}

	else if (sscanf(msg, "ping %u\n", &id) == 1){
		ping(id);
	}

	else {
		//fprintf(stderr, "Message from webpage: %s\n", msg); // debug syfte iaf man vill veta vad som fås i meddelandet
	}

}


int main(int argc, char* argv[])
{
	// Allocate
	gEngine = new sgct::Engine(argc, argv);

	// Bind your functions
	gEngine->setInitOGLFunction(myInitFun);
	gEngine->setDrawFunction(myDrawFun);
	gEngine->setPreSyncFunction(myPreSyncFun);
	gEngine->setPostSyncPreDrawFunction(myPostSyncFun);
	gEngine->setCleanUpFunction(myCleanUpFun);
	gEngine->setKeyboardCallbackFunction(keyCallback);
	sgct::SharedData::instance()->setEncodeFunction(myEncodeFun);
	sgct::SharedData::instance()->setDecodeFunction(myDecodeFun);

	// Init the engine
	if (!gEngine->init(sgct::Engine::OpenGL_3_3_Core_Profile))
	{
		delete gEngine;
		return EXIT_FAILURE;
	}

	webUsers_copy.assign(webUsers, webUsers + MAX_WEB_USERS);
	if (gEngine->isMaster())
	{
		webserver.setCallback(webDecoder);
		//webserver.start(9000);
		webserver.start(80);
		game.setGoal(); // set a random goal at the start
	}

	// Main loop
	gEngine->render();

	// Clean up (de-allocate)
	delete gEngine;

	// Exit program
	exit(EXIT_SUCCESS);

}

void myInitFun() {
	avatar.create(2.4f, 2.4f); // how big
	football.create(2.0f, 2.0f); // a football instead of a white ball ;) 
	goal.create(3.0f, 3.0f);

	loadTexturesAndStuff();

	sharedPrevGoalVec.setVal(game.getPrevGoal());
}

void myDrawFun()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	MVP = gEngine->getActiveModelViewProjectionMatrix();

	/* kanske ha en count för hur många aktiva spelare, och rendera ngt annat snygg då? hmm
	*/

	renderSkyBox();
	//renderGoal();
	renderAvatars();
	//renderFootball();

	//std::cout << "Balldirection: " << sim.GetBallDirVec().x << " ; " << sim.GetBallDirVec().y << " ; " << sim.GetBallDirVec().z << std::endl;
	//unbind shader program
	sgct::ShaderManager::instance()->unBindShaderProgram();

	glDisable(GL_BLEND);
}


void myPreSyncFun()
{
	if (gEngine->isMaster())
	{

		curr_time.setVal(static_cast<float>(sgct::Engine::getTime()));
		const float DISCONNECT_TIME = 5.0f;

		sim.Step(curr_time.getVal() - last_time.getVal());
		last_time.setVal(curr_time.getVal());
		
		sharedBallPos.setVal(sim.GetBallDirection());
		
		for (unsigned int i = 1; i<MAX_WEB_USERS; i++) {
			btVector3 pos = webUsers[i].calculatePosition();

			if (webUsers_copy[i].exists)
			{
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
		
		
		// set the goal target
		sharedGoalPos.setVal(game.getGoalQuaternion());

		sharedGoalVec.setVal(game.getGoalCoords());
		sharedBallVec.setVal(sim.GetBallDirVec());

		//copy webusers to rendering copy
		mWebMutex.lock();
		webUsers_copy.assign(webUsers, webUsers + MAX_WEB_USERS);
		mWebMutex.unlock();

		//Set the data that will be synced to the clients this frame
		sharedUserData.setVal(webUsers_copy);

		// se if goal
		if (game.update(sim.GetBallDirVec())) {
			goal_time.setVal(curr_time.getVal());
			sharedPrevGoalVec.setVal(game.getPrevGoal());
		}
		
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
		if (takeScreenShot)
		{
			gEngine->takeScreenshot();
			takeScreenShot = false;
		}
	}

}


void myEncodeFun()
{
	sgct::SharedData::instance()->writeFloat(&curr_time);
	sgct::SharedData::instance()->writeVector(&sharedUserData);
	sgct::SharedData::instance()->writeObj(&sharedBallPos);
	sgct::SharedData::instance()->writeObj(&sharedGoalPos);
	sgct::SharedData::instance()->writeObj(&sharedBallVec);
	sgct::SharedData::instance()->writeObj(&sharedGoalVec);

	sgct::SharedData::instance()->writeFloat(&goal_time);
	sgct::SharedData::instance()->writeObj(&sharedPrevGoalVec);


}

void myDecodeFun()
{
	sgct::SharedData::instance()->readFloat(&curr_time);
	sgct::SharedData::instance()->readVector(&sharedUserData);
	sgct::SharedData::instance()->readObj(&sharedBallPos);
	sgct::SharedData::instance()->readObj(&sharedGoalPos);
	sgct::SharedData::instance()->readObj(&sharedBallVec);
	sgct::SharedData::instance()->readObj(&sharedGoalVec);

	sgct::SharedData::instance()->readFloat(&goal_time);
	sgct::SharedData::instance()->readObj(&sharedPrevGoalVec);

}

void myCleanUpFun()
{
	avatar.clear();

	if (myBox != NULL)
		delete myBox;

}

void keyCallback(int key, int action)
{
	if (gEngine->isMaster())
	{
		switch (key)
		{
		case SGCT_KEY_P:
		case SGCT_KEY_F10:
			if (action == SGCT_PRESS)
				takeScreenShot = true;
			break;
		}
	}
}


// --------------------------------------------------------

void renderSkyBox()
{
	// för drawbox
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//create scene transform (animation)
	glm::mat4 scene_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

	glm::mat4 BoxMVP = MVP * scene_mat; // MVP = gEngine->getActiveModelViewProjectionMatrix()


	sgct::ShaderManager::instance()->bindShaderProgram("xform");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByHandle(textureSkyBox));

	glUniformMatrix4fv(Matrix_Loc_Box, 1, GL_FALSE, &BoxMVP[0][0]);
	glUniform1i(Tex_Loc_Box, 0);
	
	glm::vec3 v = sharedBallVec.getVal();
	//v = sim.GetBallDirVec(); // måste vara shared
	//std::cout << v.x << " " << v.y << " " << v.z << std::endl;
	glUniform3f(Ball_Vec_Loc, v.x, v.y, v.z);
	v = sharedGoalVec.getVal();
	//std::cout << v.x << " " << v.y << " " << v.z << std::endl;
	glUniform3f(Goal_Vec_Loc, v.x, v.y, v.z);
	v = sharedPrevGoalVec.getVal();
	glUniform3f(Prev_Goal_Vec_Loc, v.x, v.y, v.z);
	glUniform1f(CurrTime_Loc, curr_time.getVal());
	glUniform1f(GoalTime_Loc, goal_time.getVal());

	//draw the box (to make the texture on inside)
	glFrontFace(GL_CW);
	myBox->draw();
	glFrontFace(GL_CCW);

	sgct::ShaderManager::instance()->unBindShaderProgram();

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

}


// renderar den fina figuren som visas. 
void renderAvatars()
{

	float radius = DOME_RADIUS; //Domens radie

	glm::mat4 trans_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -radius));
	glm::vec3 color;

	sgct::ShaderManager::instance()->bindShaderProgram("avatar");
	avatar.bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByHandle(avatarTex));

	//should really look over the rendering, maybe use more threads??
	for (unsigned int i = 1; i<MAX_WEB_USERS; i++)
		if (webUsers_copy[i].exists) //sim.PlayerExists(i) ) // does it exist
		{
			btQuaternion quat = webUsers_copy[i].getPlayerDirection();//sim.GetPlayerDirection(i);
			btVector3 axis = quat.getAxis();
			float angle = quat.getAngle();
			float pingTime = webUsers_copy[i].getPingTime(); //pingedTime[i];
			float currTime = curr_time.getVal();
			int team = webUsers_copy[i].getTeam();

			if (axis.getX() == 0 && axis.getY()==0 && axis.getZ()==0) {
				// detta enbart för att det krashace på mac om detta gjordes med rotationen nedan.. hmm
			}
			
			else{
				// denna rotation kraschade förut innan if-satsen
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
				glUniform1i(Avatar_Tex_Loc, 0);

				avatar.draw();
			}

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

	btQuaternion quat = sharedBallPos.getVal();
	//std::cout << sharedBallPos.getVal().getX() << " " << sim.GetBallDirection(0) << std::endl;
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
	int team = 1; // bara så länge

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
	webUsers[id].setPingPosition(sim.GetPlayerDirVec(id));

}

void loadTexturesAndStuff(){
	// load textures
	sgct::TextureManager::instance()->setAnisotropicFilterSize(8.0f);
	sgct::TextureManager::instance()->setCompression(sgct::TextureManager::S3TC_DXT);
	sgct::TextureManager::instance()->loadTexure(avatarTex, "avatar", "avatar.png", true);
	sgct::TextureManager::instance()->loadTexure(footballTex, "fotball", "football.png", true);

	// add shaders
	sgct::ShaderManager::instance()->addShaderProgram("avatar",
		"avatar.vert",
		"avatar.frag");
	sgct::ShaderManager::instance()->bindShaderProgram("avatar");

	// avatar locs
	Matrix_Loc = sgct::ShaderManager::instance()->getShaderProgram("avatar").getUniformLocation("MVP");
	Color_Loc = sgct::ShaderManager::instance()->getShaderProgram("avatar").getUniformLocation("FaceColor");
	Avatar_Tex_Loc = sgct::ShaderManager::instance()->getShaderProgram("avatar").getUniformLocation("Tex");
	Time_Loc = sgct::ShaderManager::instance()->getShaderProgram("avatar").getUniformLocation("PingTime");
	Curr_Time = sgct::ShaderManager::instance()->getShaderProgram("avatar").getUniformLocation("CurrTime");
	Team_Loc = sgct::ShaderManager::instance()->getShaderProgram("avatar").getUniformLocation("Team");

	// fotball shader
	sgct::ShaderManager::instance()->addShaderProgram("fotball",
		"avatar.vert",
		"FootballFragShader.frag");
	sgct::ShaderManager::instance()->bindShaderProgram("fotball");

	Matrix_Loc_Football = sgct::ShaderManager::instance()->getShaderProgram("fotball").getUniformLocation("MVP");
	Tex_Loc_Football = sgct::ShaderManager::instance()->getShaderProgram("fotball").getUniformLocation("Tex");

	// goal shader
	sgct::ShaderManager::instance()->addShaderProgram("goals",
		"avatar.vert",
		"goal.frag");
	sgct::ShaderManager::instance()->bindShaderProgram("goals");

	Matrix_Loc_Goal = sgct::ShaderManager::instance()->getShaderProgram("goals").getUniformLocation("MVP");
	Team_Loc_Goal = sgct::ShaderManager::instance()->getShaderProgram("goals").getUniformLocation("Team");
	Goal_Loc_Tex = sgct::ShaderManager::instance()->getShaderProgram("goals").getUniformLocation("Tex");

	// for the skyBox
	sgct::TextureManager::instance()->loadTexure(textureSkyBox, "skyBox", "sky.png", true);

	// add the box
	myBox = new sgct_utils::SGCTBox(2.0f, sgct_utils::SGCTBox::SkyBox);

	//Set up backface culling
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW); //our polygon winding is counter clockwise

	sgct::ShaderManager::instance()->addShaderProgram("xform",
		"SimpleVertexShader.vert",
		"skyBox.frag");

	sgct::ShaderManager::instance()->bindShaderProgram("xform");

	//advanced skybox
	Matrix_Loc_Box = sgct::ShaderManager::instance()->getShaderProgram("xform").getUniformLocation("MVP");
	Tex_Loc_Box = sgct::ShaderManager::instance()->getShaderProgram("xform").getUniformLocation("Tex");
	Goal_Vec_Loc = sgct::ShaderManager::instance()->getShaderProgram("xform").getUniformLocation("GoalVec");
	Prev_Goal_Vec_Loc = sgct::ShaderManager::instance()->getShaderProgram("xform").getUniformLocation("PrevGoalVec");
	CurrTime_Loc = sgct::ShaderManager::instance()->getShaderProgram("xform").getUniformLocation("CurrTime");
	GoalTime_Loc = sgct::ShaderManager::instance()->getShaderProgram("xform").getUniformLocation("GoalTime");
	Ball_Vec_Loc = sgct::ShaderManager::instance()->getShaderProgram("xform").getUniformLocation("BallVec");

	sgct::ShaderManager::instance()->unBindShaderProgram();
}
