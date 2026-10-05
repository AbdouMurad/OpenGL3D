#pragma once

#include "Collider.h"
#include "RigidBody.h"
#include <vector>

struct ContactPoint {
	glm::vec3 position = glm::vec3(0);
	float penetration = 0.0f;
};

// Narrow phase output. All points share one normal, which points from a to b.
struct Result {
	static constexpr int MaxContacts = 4;

	ColliderComponent* a = nullptr;
	ColliderComponent* b = nullptr;

	glm::vec3 normal = glm::vec3(0);
	ContactPoint contacts[MaxContacts];
	int contactCount = 0;

	void AddContact(const glm::vec3& position, float penetration) {
		if (contactCount < MaxContacts) contacts[contactCount++] = { position, penetration };
	}
};

// Sequential impulse solver (Erin Catto, "Iterative Dynamics with Temporal Coherence").
// Each iteration nudges velocities towards satisfying every contact; repeated iterations
// let impulses propagate through stacks. Only velocities are changed here, positions are
// integrated afterwards by PhysicsWorld.
class ContactSolver {
public:
	struct Point {
		glm::vec3 rA = glm::vec3(0); // contact point relative to each center of mass
		glm::vec3 rB = glm::vec3(0);

		// 1 / (effective mass) along each direction, precomputed since bodies don't move during solving
		float normalMass = 0.0f;
		float tangentMass[2] = {};

		// total impulse applied so far this step, clamped as a whole (not per iteration)
		float normalImpulse = 0.0f;
		float tangentImpulse[2] = {};

		// target separating velocity from restitution or penetration recovery
		float bias = 0.0f;
	};

	struct Constraint {
		RigidBodyComponent* bodyA = nullptr;
		RigidBodyComponent* bodyB = nullptr;

		float inverseMassA = 0.0f;
		float inverseMassB = 0.0f;
		glm::mat3 inverseInertiaA = glm::mat3(0);
		glm::mat3 inverseInertiaB = glm::mat3(0);

		glm::vec3 normal = glm::vec3(0);
		glm::vec3 tangent[2] = {};
		float friction = 0.0f;

		Point points[Result::MaxContacts];
		int pointCount = 0;
	};

	void Prepare(const std::vector<Result>& manifolds, float dt);
	void SolveVelocities();

private:
	static void ApplyImpulse(Constraint& c, const Point& p, const glm::vec3& impulse);

	std::vector<Constraint> constraints;
};
