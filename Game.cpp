#include "Game.h"
#include <ctime>

Game::Game(){
	running = false;
	//setGoal(); // set random goal...
	setGoal(glm::vec3(7.4, 3.0, 0)); // 
	teamGoal = 0;
	/* initialize random seed: */
	prevGoal = glm::vec3(0.0, -7.4, 0);
  	srand (time(NULL));
  	teamScore[0] = 0;
  	teamScore[1] = 0;
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

bool Game::update(glm::vec3 ballCoord){ // skicka in bollens kooordinat här! ska vara normalizerad!!
	// check state in the game, if we have reached the goal. 

	// check the ballCord minus the goalCoords, if they are almost the same, GOAL! and the game ends..
	if ( glm::distance(ballCoord, goalCoords) < 0.13 ) {
		float dist = float (glm::distance(ballCoord, goalCoords)); 
		fprintf(stderr, "Goal distance %f\n", dist ); 
		reset();
		return true;
	}
	else {
		return false;

	}
}

void Game::reset(){
	// caculate points
	// set a new random goal
	setGoal();
	teamScore[teamGoal]++;
	//printScore(); 
	teamGoal = (teamGoal == 0 ? 1 : 0); 
}

int Game::getTeam(){
	return teamGoal;
}

void Game::setGoal(glm::vec3 coords){
	goalCoords = glm::normalize(coords);

}

void Game::printScore(){
	fprintf(stderr, "Points team 0: %u. Points team 1: %u\n", teamScore[0] , teamScore[1] );
}

void Game::setGoal(){
	float x = rand() % 6 + 1.4;
	float y = rand() % 6 + 1.4;
	float z = rand() % 6 + 1.4;
	x = 2.0f*x - 7.4;
	z = 2.0f*z - 7.4;
	
	prevGoal = goalCoords;
	goalCoords = glm::normalize(glm::vec3(x, y, z));

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

glm::vec3 Game::getPrevGoal() {
	return prevGoal;
}


