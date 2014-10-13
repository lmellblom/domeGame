#include "PlayerObject.h"
#include <iostream>

PlayerObject::PlayerObject(btDiscreteDynamicsWorld* world, btVector3 pos)
: Ball(world, pos, 0.35, 1){
}

PlayerObject::~PlayerObject()  {

}

void PlayerObject::Update(float dt) {
	const float MAX_SPEED = 10.0;
	const float FORCE = 10.0/dt;
	
	btVector3 position = body_->getWorldTransform().getOrigin();
	btVector3 direction = target_ - position;

	position.normalize();
	btVector3 force_direction = direction - direction.dot(position) * position;
	force_direction.normalize();
	force_direction = direction.normalize();
	
	body_->applyCentralForce(FORCE*force_direction);

	float speed = body_->getLinearVelocity().length();

	if (speed > MAX_SPEED) {
		body_->setLinearVelocity(MAX_SPEED/speed*body_->getLinearVelocity());
	}
}

void PlayerObject::SetTarget(btVector3 v) {
	target_ = v;
	v = body_->getWorldTransform().getOrigin();
	//std::cout << "Positi: " << v.getX() << " " << v.getY() << " " << v.getZ() << std::endl;
}