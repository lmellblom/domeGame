#include "PlayerObject.h"
#include <iostream>

PlayerObject::PlayerObject(btDiscreteDynamicsWorld* world, btVector3 pos)
: Ball(world, pos, 0.2, 1){
}

PlayerObject::~PlayerObject()  {

}

void PlayerObject::Update(float dt) {
	
	const float MAX_SPEED = 10.0;
	const float FORCE = 10.0/dt;
	
	btVector3 position = body_->getWorldTransform().getOrigin();
	btVector3 direction = target_ - position;
	float distance2 = direction.length2();
	direction.normalize();
	
	position.normalize();
	btVector3 force_direction = direction - direction.dot(position) * position;
	force_direction.normalize();
	
	//body_->applyCentralForce(FORCE*force_direction);

	float speed = body_->getLinearVelocity().length();

	if (distance2 < 0.1){
		body_->setLinearVelocity(btVector3(0.0,0.0,0.0));
	}
	else{
		body_->setLinearVelocity((MAX_SPEED)* force_direction);
	}
}

void PlayerObject::SetTarget(btVector3 v) {
	target_ = v;
	v = body_->getWorldTransform().getOrigin();
	//std::cout << "Positi: " << v.getX() << " " << v.getY() << " " << v.getZ() << std::endl;
}