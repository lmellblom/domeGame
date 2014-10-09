#ifndef Simulation_H
#define Simulation_H

#include <btBulletDynamicsCommon.h>

#include "sgct.h"

#define MAX_OBJECTS 256

class Simulation {
public:
	Simulation();
	~Simulation();

	void Step(float dt);
	void SetObjectTarget(int i, const btVector3& v);
	void RemoveObject(int i);
	glm::mat4 GetObjectTransform(int i);
	btQuaternion GetObjectDirection(int i);


private:
	btBroadphaseInterface* broad_phase_;
	btDefaultCollisionConfiguration* collision_configuration_;
	btCollisionDispatcher* dispatcher_;
	btSequentialImpulseConstraintSolver* solver_;
	btDiscreteDynamicsWorld* dynamics_world_;

	btCollisionShape* ground_shape_;
	btDefaultMotionState* ground_motion_state_;
	btRigidBody* ground_rigid_body_;

	btCollisionShape* sphere_shape_;
	btRigidBody* object_list_[MAX_OBJECTS];
	btPoint2PointConstraint* constraint_list_[MAX_OBJECTS];
};

#endif  // Simulation_H