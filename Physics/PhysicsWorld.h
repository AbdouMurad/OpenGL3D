#pragma once

#include "RigidBody.h"
#include "Collider.h"
#include "ContactSolver.h"

#include "Events/EventBus.h"
#include <unordered_set>

class DebugRenderer;

inline void Print(const glm::vec3& vector) {
	std::cout << vector.x << ' ' << vector.y << ' ' << vector.z << '\n';
}

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

	void ClosestPointsBetweenEdges(const glm::vec3& p1, const glm::vec3& q1, const glm::vec3& p2, const glm::vec3& q2, glm::vec3& c1, glm::vec3& c2);
	glm::vec3 FurthestPoint(const OBB& box, const glm::vec3& direction);
	std::vector<glm::vec3> ClipPolygon(const std::vector<glm::vec3>& polygon, const glm::vec3& planeNormal, float planeDistance);
	std::vector<glm::vec3> GetFaceVertices(const OBB& box, const glm::vec3& normal);
	std::vector<ContactPoint> GenerateManifold(const OBB& box1, const OBB& box2, const glm::vec3& normal, int bestAxisIndex);
	std::vector<ContactPoint> ReduceManifold(const std::vector<ContactPoint>& points, const glm::vec3& normal);

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


struct RaycastHit {
	ColliderComponent* collider = nullptr;
	glm::vec3 point = glm::vec3(0);
	glm::vec3 normal = glm::vec3(0);
	float distance = 0.0f;
};

class PhysicsWorld {
	void IntegrateVelocity(RigidBodyComponent* body, float dt);
	void IntegratePosition(RigidBodyComponent* body, float dt);
	void BuildBroad();
	void BroadPhase();
	void NarrowPhase();
	void UpdateTrigger();

	EventBus& eventBus;
	ContactSolver solver;
	std::vector<Result> contacts; //kept after Step for debug drawing
public:
	void DebugRender(DebugRenderer& renderer);
	PhysicsWorld(EventBus&);

	//change to 2 buffers later and instead of copying one to the other just change which is which
	std::unordered_set<Pair, PairHash> triggerBuffer1;
	std::unordered_set<Pair, PairHash> triggerBuffer2;
	bool triggerBufferSwitch = false; //false -> buffer 1 = current, buffer 2 = previous. true -> opposite;

	std::vector<Pair> broadPhase;
	std::vector<RigidBodyComponent*> bodies;
	std::vector<ColliderComponent*> colliders;
	glm::vec3 gravity = glm::vec3(0, -9.81, 0);
	int solverIterations = 10;
	void Step(float dt);

	//closest non-trigger collider along the ray. direction must be normalized, colliders owned by ignore are skipped
	bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit& hit, const GameObject* ignore = nullptr) const;

	//drops an object's bodies and colliders before it's deleted. Trigger pairs it was part of get their exit event now,
	//while the object is still alive, instead of later with a dangling pointer
	void RemoveObject(GameObject& object);
};