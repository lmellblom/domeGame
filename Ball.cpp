#include "Ball.h"
#include <iostream>

Ball::Ball(btDiscreteDynamicsWorld* world, btVector3 pos, float radius, float mass) {
	world_ = world;

	shape_ = new btSphereShape(radius);

	btDefaultMotionState* fallMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), pos));
	btVector3 fallInertia(0, 0, 0);
	shape_->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, fallMotionState, shape_, fallInertia);
	body_ = new btRigidBody(rigidBodyCI);
	world_->addRigidBody(body_);
	body_->setActivationState(DISABLE_DEACTIVATION);
	constraint_ = new btPoint2PointConstraint(*body_, -pos);
	world_->addConstraint(constraint_);

}

Ball::~Ball()  {
	world_->removeConstraint(constraint_);
	delete constraint_;

	world_->removeRigidBody(body_);
	delete body_->getMotionState();
	delete body_;
	delete shape_;
}

btVector3 Ball::GetDirection() {
	btVector3 pos = body_->getWorldTransform().getOrigin();
	return pos.normalize();
}