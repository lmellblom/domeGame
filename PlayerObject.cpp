#include "PlayerObject.h"
#include <iostream>

PlayerObject::PlayerObject(btDiscreteDynamicsWorld* world, btVector3 pos) {
	world_ = world;

	shape_ = new btSphereShape(0.35);

	btDefaultMotionState* fallMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), pos));
	btScalar mass = 1;
	btVector3 fallInertia(0, 0, 0);
	shape_->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, fallMotionState, shape_, fallInertia);
	body_ = new btRigidBody(rigidBodyCI);
	world_->addRigidBody(body_);
	body_->setActivationState(DISABLE_DEACTIVATION);
	constraint_ = new btPoint2PointConstraint(*body_, -pos);
	world_->addConstraint(constraint_);

}

PlayerObject::~PlayerObject()  {
	world_->removeConstraint(constraint_);
	delete constraint_;

	world_->removeRigidBody(body_);
	delete body_->getMotionState();
	delete body_;
	delete shape_;
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

btVector3 PlayerObject::GetDirection() {
	btVector3 pos = body_->getWorldTransform().getOrigin();
	return pos.normalize();
}