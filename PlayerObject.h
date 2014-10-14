#ifndef PlayerObject_H
#define PlayerObject_H

#include <btBulletDynamicsCommon.h>

#include "sgct.h"
#include "Ball.h"

#define MAX_OBJECTS 256

class PlayerObject: public Ball {
public:
	PlayerObject(btDiscreteDynamicsWorld* world, btVector3 pos);
	~PlayerObject();

	void Update(float dt);
	void SetTarget(btVector3 v);


private:
	btVector3 target_;
};

#endif  // PlayerObject_H