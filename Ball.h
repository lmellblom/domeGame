#ifndef Ball_H
#define Ball_H

#include <btBulletDynamicsCommon.h>

#include "sgct.h"

#define MAX_OBJECTS 256

class Ball {
public:
	Ball(btDiscreteDynamicsWorld* world, btVector3 pos, float radius, float mass);
	~Ball();

	btVector3 GetDirection();


protected:
	btDiscreteDynamicsWorld* world_;
	btCollisionShape* shape_;
	btRigidBody* body_;
	btPoint2PointConstraint* constraint_;
};

#endif  // Ball_H