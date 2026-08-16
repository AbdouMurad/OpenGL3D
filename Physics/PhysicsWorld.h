#pragma once

#include "RigidBody.h"
#include "Collider.h"

#include "Events/EventBus.h"
#include <unordered_set>

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
	using BroadCollisionFn = AABB(*)(ColliderComponent&);

	constexpr int ShapeCount = static_cast<int>(SHAPE::SIZE);

	extern BroadCollisionFn broadDispatchTable[ShapeCount];
	extern CollisionFn dispatchTable[ShapeCount][ShapeCount];

	void init();

	AABB BoxShape(ColliderComponent&);
	AABB SphereShape(ColliderComponent&);
	//AABB CapsuleShape(ColliderComponent&);

	bool SphereSphere(ColliderComponent&, ColliderComponent&, Result&);
	bool SphereBox(ColliderComponent&, ColliderComponent&, Result&);
	bool BoxSphere(ColliderComponent&, ColliderComponent&, Result&);
	bool BoxBox(ColliderComponent&, ColliderComponent&, Result&);
	//bool CapsuleBox(ColliderComponent&, ColliderComponent&, Result&);
	//bool BoxCapsule(ColliderComponent&, ColliderComponent&, Result&);
	//bool CapsuleSphere(ColliderComponent&, ColliderComponent&, Result&);
	//bool SphereCapsule(ColliderComponent&, ColliderComponent&, Result&);
	//bool CapsuleCapsule(ColliderComponent&, ColliderComponent&, Result&);



	bool AABBTest(const AABB&, const AABB&);
	bool Test(ColliderComponent& a, ColliderComponent& b, Result& result);
}

struct Pair {
	ColliderComponent* a;
	ColliderComponent* b;

	bool operator==(const Pair& other) const
	{
		return (a == other.a && b == other.b) || (a == other.b && b == other.a);
	}
	

};
struct PairHash {
	//used to hash pair -> for set
	size_t operator()(const Pair& pair) const {
		auto a = std::hash<const ColliderComponent*>{}(pair.a);
		auto b = std::hash<const ColliderComponent*>{}(pair.b);
		return a ^ b;
	}
};


class PhysicsWorld {
	void Integrate(RigidBodyComponent* body, float dt);
	void BuildBroad();
	void BroadPhase();
	void CollisionCheck();
	void HandleCollision(Result& result);
	void ResolveCollision(RigidBodyComponent*, RigidBodyComponent*, Result& result);
	void UpdateTrigger();
	
	EventBus& eventBus;
public:
	PhysicsWorld(EventBus&);

	//change to 2 buffers later and instead of copying one to the other just change which is which
	std::unordered_set<Pair, PairHash> triggerBuffer1;
	std::unordered_set<Pair, PairHash> triggerBuffer2;
	bool triggerBufferSwitch = false; //false -> buffer 1 = current, buffer 2 = previous. true -> opposite;

	std::vector<Pair> broadPhase;
	std::vector<RigidBodyComponent*> bodies;
	std::vector<ColliderComponent*> colliders;
	glm::vec3 gravity = glm::vec3(0, -9.81, 0);
	void Step(float dt);
};