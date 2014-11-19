#include "Game.h"

Game::Game(){
	running = false;
	//setGoal(); // set random goal...
	setGoal(glm::vec3(7.4, 3.0, 0)); // 
}

Game::~Game(){

}

void Game::init(){
}

void Game::start(){
	running = true; 
}

void Game::addPlayer(){
}

void Game::update(glm::vec3 ballCoord){ // skicka in bollens kooordinat här! ska vara normalizerad!!
	// check state in the game, if we have reached the goal. 

	// check the ballCord minus the goalCoords, if they are almost the same, GOAL! and the game ends..

	if ( glm::distance(ballCoord, goalCoords) < 0.13 ) {
		float dist = float (glm::distance(ballCoord, goalCoords)); 
		fprintf(stderr, "Goal distance %f\n", dist ); 
	}


}

void Game::reset(){

}

void Game::setGoal(glm::vec3 coords){
	goalCoords = glm::normalize(coords);

}


glm::vec3 Game::getGoalCoords(){
	return goalCoords;
}

btQuaternion Game::getGoalQuaternion(){
	btVector3 up = btVector3(0, 0, -1);
	btVector3 dir = btVector3(goalCoords.x, goalCoords.y, goalCoords.z).normalize();
	btVector3 xyz = up.cross(dir);
	float w = sqrt(up.length2() * dir.length2()) + up.dot(dir);
	return btQuaternion(xyz.getX(), xyz.getY(), xyz.getZ(), w).normalize();
}





