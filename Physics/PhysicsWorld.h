#pragma once

#include "RigidBody.h"
#include "Collider.h"

#include "Events/EventBus.h"
#include <unordered_set>
#include <unordered_map>
#include <cfloat>

inline void Print(const glm::vec3& vector) {
	std::cout << vector.x << ' ' << vector.y << ' ' << vector.z << '\n';
}

//one point where two colliders touch
struct ContactPoint {
	glm::vec3 position = glm::vec3(0);
	float penetration = 0.0f;
};

struct Result {
	ColliderComponent* a = nullptr;
	ColliderComponent* b = nullptr;

	glm::vec3 normal = glm::vec3(0);

	//1 for spheres and edge-edge, up to 8 for box faces (corners of the clipped touching area)
	//several points let a flat face push back harder on its lower side -> resists tilting
	static constexpr int MaxPoints = 8;
	ContactPoint points[MaxPoints];
	int pointCount = 0;
	void AddPoint(const glm::vec3& position, float penetration) {
		if (pointCount < MaxPoints) points[pointCount++] = { position, penetration };
	}
};

//what a raycast hit
struct RaycastHit {
	ColliderComponent* collider = nullptr; //-> getOwner() for the object, GetRigidBody() for physics
	glm::vec3 point = glm::vec3(0);		   //world space
	glm::vec3 normal = glm::vec3(0);	   //surface direction at the hit (faces back towards the ray)
	float distance = 0.0f;				   //metres from the origin (the ray's t, with a normalized direction)
};
std::ostream& operator<<(std::ostream& os, const RaycastHit& hit);

//optional filters for a raycast
struct RaycastOptions {
	float maxDistance = FLT_MAX;		//how far to look, infinite by default
	bool hitTriggers = false;			//trigger zones are usually invisible -> skipped
	const GameObject* ignore = nullptr; //skip this object's colliders (e.g. whoever is shooting)
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
	std::vector<ContactPoint> GenerateManifold(const OBB& box1, const OBB& box2, const glm::vec3& normal, int bestAxisIndex);

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



	//ray (normalized direction) against one collider, closer than maxDistance
	//false on a miss, or when the origin is inside the collider (it's skipped)
	bool RaySphere(const glm::vec3& origin, const glm::vec3& direction, ColliderComponent& collider, float maxDistance, RaycastHit& hit);
	bool RayBox(const glm::vec3& origin, const glm::vec3& direction, ColliderComponent& collider, float maxDistance, RaycastHit& hit);
	bool Ray(const glm::vec3& origin, const glm::vec3& direction, ColliderComponent& collider, float maxDistance, RaycastHit& hit);

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
	glm::vec3 position = glm::vec3(0); //world space contact point (matches it to last step's point for warm starting)
	glm::vec3 rA = glm::vec3(0);	 //contact point relative to each body's centre
	glm::vec3 rB = glm::vec3(0);
	glm::vec3 centerA = glm::vec3(0); //centres of mass, shared by every point of the pair
	glm::vec3 centerB = glm::vec3(0);
	float penetration = 0.0f;
	float positionWeight = 1.0f;	 //1 / points in this pair -> the pair corrects its overlap once, not once per point

	float inverseMassA = 0.0f;
	float inverseMassB = 0.0f;
	glm::mat3 inverseInertiaA = glm::mat3(0); //world space, rotation locks applied
	glm::mat3 inverseInertiaB = glm::mat3(0);

	//false -> static or rotation frozen: the solver skips every angular term for that body
	bool rotatesA = false;
	bool rotatesB = false;

	float normalDenominator = 0.0f; //inverse masses + rotational terms along the normal
	float bounceVelocity = 0.0f;	//separating speed the solver aims for (restitution), 0 = no bounce
	float friction = 0.0f;			//combined from both bodies

	//friction works along two fixed directions on the surface (fixed so their running totals mean something)
	glm::vec3 tangent[2] = { glm::vec3(0), glm::vec3(0) };
	float tangentDenominator[2] = { 0.0f, 0.0f };

	//accumulated impulses: running totals of what this contact has applied this step
	//the solver clamps the TOTAL and applies only the change -> a pass can ease back an earlier over-push
	float normalImpulse = 0.0f;					//>= 0, contacts only push
	float tangentImpulse[2] = { 0.0f, 0.0f };	//within +-friction * normalImpulse

	Pair pair = { nullptr, nullptr };			//the colliders touching -> finds last step's totals
};

//a contact point's totals kept for the next step (warm starting)
struct CachedPoint {
	glm::vec3 position = glm::vec3(0);
	float normal = 0.0f;
	glm::vec3 friction = glm::vec3(0); //world space, so it survives the tangents being rebuilt
};
//all points of one touching pair
struct CachedManifold {
	ColliderComponent* a = nullptr; //pair order it was stored with (normal points from a to b)
	CachedPoint points[Result::MaxPoints];
	int pointCount = 0;
};

class PhysicsWorld {
	void Integrate(RigidBodyComponent* body, float dt);
	void BuildBroad();
	void BroadPhase();
	void DetectContacts();
	bool SetupBodies(const Result& result, Contact& contact);
	void SetupPoint(const ContactPoint& point, Contact& contact);
	void SolveContact(Contact& contact);
	void ApplyImpulse(const Contact& contact, const glm::vec3& impulse);
	void WarmStart(Contact& contact);
	void StoreImpulses();
	void CorrectPosition(const Contact& contact);
	void UpdateTrigger();

	std::vector<Contact> contacts;
	std::unordered_map<Pair, CachedManifold, PairHash> impulseCache; //last step's totals per touching pair


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

	//closest collider along the ray. direction doesn't need to be normalized. false = hit nothing
	bool Raycast(const glm::vec3& origin, const glm::vec3& direction, RaycastHit& hit, const RaycastOptions& options = {}) const;
};