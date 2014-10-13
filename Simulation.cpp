#include "Simulation.h"
#include "PlayerObject.h"
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
	//dynamics_world_->setGravity(btVector3(0, -9.82, 0));

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

	ball_list_[0] = new Ball(dynamics_world_, btVector3(0.0, 7.4, 0.0), 1.0, 3.0);
}

Simulation::~Simulation()  {
	//dynamics_world_->removeRigidBody(ground_rigid_body_);
	delete ground_rigid_body_->getMotionState();
	delete ground_rigid_body_;
	delete ground_shape_;

	for (int i = 0; i < MAX_PLAYERS; i++)
		if (player_list_[i] != NULL) {
			delete player_list_[i];
		}


	delete dynamics_world_;
	delete solver_;
	delete collision_configuration_;
	delete dispatcher_;
	delete broad_phase_;
}


void Simulation::Step(float dt) {
	dynamics_world_->stepSimulation(dt, 5, 1.0/60.0);

	for (int i = 0; i < MAX_PLAYERS; i++)
		if (player_list_[i] != NULL) {
			player_list_[i]->Update(dt);
		}
}

//not very good solution, should be called at a fixed interval
void Simulation::SetPlayerTarget(int i, const btVector3& v) {
	if (player_list_[i] != NULL) {
		player_list_[i]->SetTarget(v);
	}
	else {
		player_list_[i] = new PlayerObject(dynamics_world_, v);
	}
	
}

void Simulation::RemovePlayer(int i) {
	if (player_list_[i] != NULL) {
		delete player_list_[i];
		player_list_[i] = NULL;
	}
}

btQuaternion Simulation::GetPlayerDirection(int i) {
	btVector3 up = btVector3(0, 0, -1);
	btVector3 dir = player_list_[i]->GetDirection();
	btVector3 xyz = up.cross(dir);
	float w = sqrt(up.length2() * dir.length2()) + up.dot(dir);
	return btQuaternion(xyz.getX(), xyz.getY(), xyz.getZ(), w).normalize();
}


bool Simulation::PlayerExists(int i) {
	return player_list_[i] != NULL;
}

btQuaternion Simulation::GetBallDirection(int i) {
	btVector3 up = btVector3(0, 0, -1);
	btVector3 dir = ball_list_[i]->GetDirection();
	btVector3 xyz = up.cross(dir);
	float w = sqrt(up.length2() * dir.length2()) + up.dot(dir);
	return btQuaternion(xyz.getX(), xyz.getY(), xyz.getZ(), w).normalize();
}