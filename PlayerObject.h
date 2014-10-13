#ifndef PlayerObject_H
#define PlayerObject_H

#include <btBulletDynamicsCommon.h>

#include "sgct.h"

#define MAX_OBJECTS 256

class PlayerObject {
public:
	PlayerObject(btDiscreteDynamicsWorld* world, btVector3 pos);
	~PlayerObject();

	void Update(float dt);
	void SetTarget(btVector3 v);
	btVector3 GetDirection();


private:
	btVector3 target_;
	btDiscreteDynamicsWorld* world_;
	btCollisionShape* shape_;
	btRigidBody* body_;
	btPoint2PointConstraint* constraint_;
};

#endif  // PlayerObject_H