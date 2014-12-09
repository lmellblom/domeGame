#ifndef Simulation_H
#define Simulation_H

#include <btBulletDynamicsCommon.h>

#include "sgct.h"
#include "PlayerObject.h"

#define MAX_PLAYERS 64
#define MAX_STUFF 10

class Simulation {
public:
	Simulation();
	~Simulation();

	void Step(float dt);
	void SetPlayerTarget(int i, const btVector3& v);
	void RemovePlayer(int i);
	btQuaternion GetPlayerDirection(int i);
	glm::vec3 GetPlayerDirVec(int i);
	bool PlayerExists(int i);

	btQuaternion GetBallDirection();
	glm::vec3 GetBallDirVec();


private:
	btBroadphaseInterface* broad_phase_;
	btDefaultCollisionConfiguration* collision_configuration_;
	btCollisionDispatcher* dispatcher_;
	btSequentialImpulseConstraintSolver* solver_;
	btDiscreteDynamicsWorld* dynamics_world_;

	btCollisionShape* ground_shape_;
	btDefaultMotionState* ground_motion_state_;
	btRigidBody* ground_rigid_body_;

	PlayerObject* player_list_[MAX_PLAYERS];
	Ball* ball;
};

#endif  // Simulation_H