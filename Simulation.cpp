#include "Simulation.h"
#include <iostream>

Simulation::Simulation() {
	//init basic bullet stuff
	broad_phase_ = new btDbvtBroadphase();
	collision_configuration_ = new btDefaultCollisionConfiguration();
	dispatcher_ = new btCollisionDispatcher(collision_configuration_);
	solver_ = new btSequentialImpulseConstraintSolver;

	dynamics_world_ = new btDiscreteDynamicsWorld(dispatcher_,
		broad_phase_, solver_, collision_configuration_);

	//set gravity
	dynamics_world_->setGravity(btVector3(0, -9.82, 0));

	// Init game object
	game_object_ = new btSphereShape(0.1);
	btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 10, 0)));
	btScalar mass = 1;
	btVector3 fallInertia(0, 0, 0);
	game_object_->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, game_object_, fallInertia);
	btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
	dynamics_world_->addRigidBody(fallRigidBody);

	//init ground object
	ground_shape_ = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
	ground_motion_state_ =
		new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1),
		btVector3(0, 0, 0)));

	btRigidBody::btRigidBodyConstructionInfo ground_rigid_bodyCI(0,
		ground_motion_state_, ground_shape_, btVector3(0, 0, 0));

	ground_rigid_body_ = new btRigidBody(ground_rigid_bodyCI);
	ground_rigid_body_->setFriction(1.0f);

	//dynamics_world_->addRigidBody(ground_rigid_body_);

	//other
	sphere_shape_ = new btSphereShape(0.04); // radius to the shape.. påverkar hur nära de kan vara.. bara höftat värdet

}

Simulation::~Simulation()  {
	//dynamics_world_->removeRigidBody(ground_rigid_body_);
	delete ground_rigid_body_->getMotionState();
	delete ground_rigid_body_;
	delete ground_shape_;
	delete game_object_;

	for (int i = 0; i < MAX_OBJECTS; i++) {
		dynamics_world_->removeRigidBody(object_list_[i]);
		delete object_list_[i]->getMotionState();
		delete object_list_[i];
	}
	delete sphere_shape_;

	
	delete dynamics_world_;
	delete solver_;
	delete collision_configuration_;
	delete dispatcher_;
	delete broad_phase_;
}


void Simulation::Step(float dt) {
	dynamics_world_->stepSimulation(dt, 5, 1.0/60.0);
}

//not very good solution, should be called at a fixed interval
void Simulation::SetObjectTarget(int i, const btVector3& v) {
	
	//constants choosen at random sorta
	const float MAX_SPEED = 1.0;
	const float IMPULSE_FORCE = 500.0;

	if (object_list_[i] != NULL) {
		btVector3 current_position = object_list_[i]->getWorldTransform().getOrigin();
		
		//the direction is straight to the target. Should probably be along the plane by projecting the direction vector in the normal plane for current_position
		btVector3 direction = v - current_position;
		direction.normalize();

		//object_list_[i]->applyCentralImpulse(IMPULSE_FORCE*direction);
		float speed = object_list_[i]->getLinearVelocity().length();
		object_list_[i]->setLinearVelocity(MAX_SPEED*direction);
		btVector3 vec = current_position;
		//std::cout << vec.getX() << " " << vec.getY() << " " << vec.getZ() << std::endl;
		/*
		btTransform transform = object_list_[i]->getCenterOfMassTransform();
		transform.setOrigin(v);
		object_list_[i]->setCenterOfMassTransform(transform);
		*/
		
	}
	else { //create body if it doesn't exist
		const float RADIUS = 7.4f;

		btVector3 position = btVector3(0, RADIUS, 0);
		btDefaultMotionState* fallMotionState =
			new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), position));
		btScalar mass = 1;
		btVector3 fallInertia(0, 0, 0);
		sphere_shape_->calculateLocalInertia(mass, fallInertia);
		btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, fallMotionState, sphere_shape_, fallInertia);
		object_list_[i] = new btRigidBody(rigidBodyCI);
		dynamics_world_->addRigidBody(object_list_[i]);
		object_list_[i]->setActivationState(DISABLE_DEACTIVATION);

		constraint_list_[i] = new btPoint2PointConstraint(*ground_rigid_body_, *object_list_[i], btVector3(0, 0, 0), -position);
	}
	
}

void Simulation::RemoveObject(int i) {
	
	if (object_list_[i] != NULL) {
		delete constraint_list_[i];
		dynamics_world_->removeRigidBody(object_list_[i]);
		delete object_list_[i]->getMotionState();
		delete object_list_[i];
		object_list_[i] = NULL;
	}
	
}

glm::mat4 Simulation::GetObjectTransform(int i) {
	
	btTransform transform = object_list_[i]->getWorldTransform();
	glm::mat4 matrix;
	transform.getOpenGLMatrix(glm::value_ptr(matrix));
	return matrix;
	
}

btQuaternion Simulation::GetObjectDirection(int i) {
	btVector3 up = btVector3(0, 0, -1);
	btVector3 dir = object_list_[i]->getWorldTransform().getOrigin().normalize();
	btVector3 xyz = up.cross(dir);
	float w = sqrt(up.length2() * dir.length2()) + up.dot(dir);
		return btQuaternion(xyz.getX(), xyz.getY(), xyz.getZ(), w).normalize();
}