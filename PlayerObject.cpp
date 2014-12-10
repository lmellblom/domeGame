#include "PlayerObject.h"
#include <iostream>

PlayerObject::PlayerObject(btDiscreteDynamicsWorld* world, btVector3 pos)
: Ball(world, pos, 0.2, 1){
}

PlayerObject::~PlayerObject()  {

}

void PlayerObject::Update(float dt) {
	
	const float MAX_SPEED = 10.0;
	const float FORCE = 0.0;
	
	btVector3 position = body_->getWorldTransform().getOrigin();
	btVector3 direction = target_ - position;
	float distance2 = direction.length2();
	direction.normalize();
	//std::cout << "t" << target_.length() << std::endl;
	//std::cout << "p" << position.length() << std::endl;
	position.normalize();
	btVector3 force_direction = direction - direction.dot(position) * position;
	force_direction.normalize();
	
	//body_->applyCentralForce(FORCE*force_direction);

	
	if (distance2 < 5.0){
		body_->setLinearVelocity((MAX_SPEED) * distance2/5.0 * force_direction);
	}
	else{
		body_->setLinearVelocity((MAX_SPEED) * force_direction);
	}

}

void PlayerObject::SetTarget(btVector3 v) {
	target_ = v;
	v = body_->getWorldTransform().getOrigin();
	//std::cout << "Positi: " << v.getX() << " " << v.getY() << " " << v.getZ() << std::endl;
}