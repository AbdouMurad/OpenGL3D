#pragma once

#include "RigidBody.h"
#include "Collider.h"

#include "Events/EventBus.h"
#include <unordered_set>

inline void Print(const glm::vec3& vector) {
	std::cout << vector.x << ' ' << vector.y << ' ' << vector.z << '\n';
}

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

	void ClosestPointsBetweenEdges(const glm::vec3& p1, const glm::vec3& q1, const glm::vec3& p2, const glm::vec3& q2, glm::vec3& c1, glm::vec3& c2);
	glm::vec3 FurthestPoint(const OBB& box, const glm::vec3& direction);
	std::vector<glm::vec3> ClipPolygon(const std::vector<glm::vec3>& polygon, const glm::vec3& planeNormal, float planeDistance);
	std::vector<glm::vec3> GetFaceVertices(const OBB& box, const glm::vec3& normal);
	std::vector<glm::vec3> GenerateManifold(const OBB& box1, const OBB& box2, const glm::vec3& normal, int bestAxisIndex);

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


//one solid contact, built once per step by collision detection
//geometry + masses are fixed for the step -> only velocities change while the solver iterates
struct Contact {
	RigidBodyComponent* a = nullptr; //null or static -> immovable
	RigidBodyComponent* b = nullptr;

	glm::vec3 normal = glm::vec3(0); //points from a to b
	glm::vec3 rA = glm::vec3(0);	 //contact point relative to each body's centre
	glm::vec3 rB = glm::vec3(0);
	float penetration = 0.0f;

	float inverseMassA = 0.0f;
	float inverseMassB = 0.0f;
	glm::mat3 inverseInertiaA = glm::mat3(0); //world space, rotation locks applied
	glm::mat3 inverseInertiaB = glm::mat3(0);

	float normalDenominator = 0.0f; //inverse masses + rotational terms along the normal
};

class PhysicsWorld {
	void Integrate(RigidBodyComponent* body, float dt);
	void BuildBroad();
	void BroadPhase();
	void DetectContacts();
	bool BuildContact(const Result& result, Contact& contact);
	void SolveContact(Contact& contact);
	void CorrectPosition(const Contact& contact);
	void UpdateTrigger();

	std::vector<Contact> contacts;


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
	void Clear();
};