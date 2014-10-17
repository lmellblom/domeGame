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
	void update(glm::vec3 ballCoord);
	void reset();
	void setGoal(glm::vec3 coords);
	glm::vec3 getGoalCoords();
	btQuaternion getGoalQuaternion();

private:
	int numberOfPlayers;
	bool running; // is the game running

	glm::vec3 goalCoords;

};

#endif