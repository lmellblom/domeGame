#define MAX_WEB_USERS 256
#define DOME_RADIUS 7.4f

#include "Render.h"

sgct::Engine * gEngine;

void ping(unsigned int id); // ping from user

Game game;

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

Quad avatar;
Quad ball;
Quad football;

void myInitFun()
{

	ball.create(2.0f, 2.0f);
	avatar.create(2.4f, 2.4f); // how big
	football.create(2.0f, 2.0f); // a football instead of a white ball ;) 

	for (int i = 0; i < MAX_WEB_USERS; i++) {
		pingedTime[i] = 0.0f;
		pingedPosition[i] = glm::vec3(0.f, 0.f, 0.f);
	}

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

	// for the skyBox
	sgct::TextureManager::instance()->loadTexure(textureSkyBox, "skyBox", "sky.png", true);

	// add the box
	myBox = new sgct_utils::SGCTBox(2.0f, sgct_utils::SGCTBox::SkyBox);

	//Set up backface culling
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW); //our polygon winding is counter clockwise

	sgct::ShaderManager::instance()->addShaderProgram("xform",
		"SimpleVertexShader.vert",
		"SimpleFragmentShader.frag");

	sgct::ShaderManager::instance()->bindShaderProgram("xform");

	Matrix_Loc_Box = sgct::ShaderManager::instance()->getShaderProgram("xform").getUniformLocation("MVP");
	Tex_Loc_Box = sgct::ShaderManager::instance()->getShaderProgram("xform").getUniformLocation("Tex");

	sgct::ShaderManager::instance()->unBindShaderProgram();
}

void myDrawFun()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	MVP = gEngine->getActiveModelViewProjectionMatrix();

	renderSkyBox();
	renderAvatars();
	// renderBalls();
	renderFootball();
	renderGoal();

	// check if goal
	game.update(sim.GetBallDirectionNonQuaternion(0));

	//unbind shader program
	sgct::ShaderManager::instance()->unBindShaderProgram();

	glDisable(GL_BLEND);

}

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

	//draw the box (to make the texture on inside)
	glFrontFace(GL_CW);
	myBox->draw();
	glFrontFace(GL_CCW);

	sgct::ShaderManager::instance()->unBindShaderProgram();

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

}

void renderGoal(){

	float radius = DOME_RADIUS; //Domens radie

	glm::mat4 trans_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -radius));
	glm::vec3 color;

	sgct::ShaderManager::instance()->bindShaderProgram("avatar");
	ball.bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByHandle(avatarTex));

	//should really look over the rendering
	btQuaternion quat = game.getGoalQuaternion();
	btVector3 axis = quat.getAxis();
	float angle = quat.getAngle();

	glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f),
		glm::degrees(angle),
		glm::vec3(axis.getX(), axis.getY(), axis.getZ()));

	glm::mat4 avatarMat = MVP * rot_mat * trans_mat;

	glUniformMatrix4fv(Matrix_Loc, 1, GL_FALSE, &avatarMat[0][0]);
	glUniform3f(Color_Loc, 0.0, 0.0, 0.0); // color on the goal
	glUniform1i(Avatar_Tex_Loc, 0);

	ball.draw();

	ball.unbind();
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
		if (sim.PlayerExists(i))
		{
		btQuaternion quat = sim.GetPlayerDirection(i);
		btVector3 axis = quat.getAxis();
		float angle = quat.getAngle();
		float pingTime = pingedTime[i];
		float currTime = curr_time.getVal();
		int team = webUsers[i].getTeam();

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

	avatar.unbind();
}

void renderBalls() {
	float radius = DOME_RADIUS; //Domens radie

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

void renderFootball() {
	float radius = DOME_RADIUS;
	glm::mat4 trans_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -radius));
	sgct::ShaderManager::instance()->bindShaderProgram("fotball");

	football.bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sgct::TextureManager::instance()->getTextureByHandle(footballTex));

	btQuaternion quat = sim.GetBallDirection(0);
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

void myCleanUpFun()
{
	avatar.clear();

	if (myBox != NULL)
		delete myBox;

}
