#pragma once

#include "RigidBody.h"
#include "Collider.h"

struct Result {
	ColliderComponent* a = nullptr;
	ColliderComponent* b = nullptr;

	glm::vec3 normal = glm::vec3(0);
	float penetration = 0.0f;
	glm::vec3 point = glm::vec3(0);
};

namespace Collision {
	struct OBB {
		glm::vec3 center = glm::vec3(0);
		glm::vec3 right = glm::vec3(0);
		glm::vec3 up = glm::vec3(0);
		glm::vec3 forward = glm::vec3(0);

		glm::vec3 halfExtent = glm::vec3(0);
	};

	using CollisionFn = bool(*)(ColliderComponent&, ColliderComponent&, Result&); //pointer to function

	constexpr int ShapeCount = static_cast<int>(SHAPE::SIZE);
	extern CollisionFn dispatchTable[ShapeCount][ShapeCount];

	void init();

	bool SphereSphere(ColliderComponent&, ColliderComponent&, Result&);
	bool SphereBox(ColliderComponent&, ColliderComponent&, Result&);
	bool BoxSphere(ColliderComponent&, ColliderComponent&, Result&);
	bool BoxBox(ColliderComponent&, ColliderComponent&, Result&);
	//bool CapsuleBox(ColliderComponent&, ColliderComponent&, Result&);
	//bool BoxCapsule(ColliderComponent&, ColliderComponent&, Result&);
	//bool CapsuleSphere(ColliderComponent&, ColliderComponent&, Result&);
	//bool SphereCapsule(ColliderComponent&, ColliderComponent&, Result&);
	//bool CapsuleCapsule(ColliderComponent&, ColliderComponent&, Result&);

	bool Test(ColliderComponent& a, ColliderComponent& b, Result& result);
}

class PhysicsWorld {
	void Integrate(RigidBodyComponent* body, float dt);
	void CollisionCheck();
	void HandleCollision(Result& result);
	void ResolveCollision(RigidBodyComponent*, RigidBodyComponent*, Result& result);
	
public:
	std::vector<RigidBodyComponent*> bodies;
	std::vector<ColliderComponent*> colliders;
	glm::vec3 gravity = glm::vec3(0, -9.81, 0);
	void Step(float dt);
};