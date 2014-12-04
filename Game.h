#ifndef Game_H
#define Game_H
#include "sgct.h"
#include <btBulletDynamicsCommon.h>


class Game {
public:
	Game();
	~Game();

	void init();
	void start();
	void addPlayer();
	bool update(glm::vec3 ballCoord);
	void reset();
	void setGoal(glm::vec3 coords);
	void setGoal();
	int getTeam(); 
	glm::vec3 getGoalCoords();
	glm::vec3 getPrevGoal();
	btQuaternion getGoalQuaternion();
	void printScore();

private:
	int numberOfPlayers;
	bool running; // is the game running
	int teamGoal; // 0 or 1 depending on which team that are supposed to score

	//std::vector<int> teamScore; 

	int teamScore[2]; // two teams
	glm::vec3 goalCoords;
	glm::vec3 prevGoal;
};

#endif