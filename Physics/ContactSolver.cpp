#include "ContactSolver.h"

#include <algorithm>
#include <cmath>

//fraction of the penetration removed per second (Baumgarte stabilization)
static constexpr float baumgarte = 0.2f;
//penetration allowed before correcting, keeps resting contacts touching instead of flickering
static constexpr float slop = 0.01f;
//impacts slower than this don't bounce, otherwise resting objects never settle
static constexpr float restitutionThreshold = 1.0f;

static bool IsDynamic(const RigidBodyComponent* body) {
	return body && !body->IsStatic();
}

static glm::vec3 VelocityAt(const RigidBodyComponent* body, const glm::vec3& r) {
	if (!body) return glm::vec3(0);
	return body->GetVelocity() + glm::cross(body->GetAngularVelocity(), r);
}

// two tangent directions perpendicular to the normal, for friction
static void ComputeBasis(const glm::vec3& n, glm::vec3& t1, glm::vec3& t2) {
	//cross with whichever axis is far from n so the result can't be near zero
	if (std::abs(n.x) >= 0.57735f) //1/sqrt(3): at least one component is always this big
		t1 = glm::normalize(glm::vec3(n.y, -n.x, 0.0f));
	else
		t1 = glm::normalize(glm::vec3(0.0f, n.z, -n.y));
	t2 = glm::cross(n, t1);
}

// how much the relative velocity at the contact changes per unit impulse along direction, inverted
static float EffectiveMass(const ContactSolver::Constraint& c, const glm::vec3& rA, const glm::vec3& rB, const glm::vec3& direction) {
	glm::vec3 rnA = glm::cross(rA, direction);
	glm::vec3 rnB = glm::cross(rB, direction);
	float k = c.inverseMassA + c.inverseMassB
		+ glm::dot(rnA, c.inverseInertiaA * rnA)
		+ glm::dot(rnB, c.inverseInertiaB * rnB);
	return k > 0.0f ? 1.0f / k : 0.0f;
}

// impulse is applied to b, and its opposite to a
void ContactSolver::ApplyImpulse(Constraint& c, const Point& p, const glm::vec3& impulse) {
	if (IsDynamic(c.bodyA)) {
		c.bodyA->velocity -= impulse * c.inverseMassA;
		c.bodyA->angularVelocity -= c.inverseInertiaA * glm::cross(p.rA, impulse);
	}
	if (IsDynamic(c.bodyB)) {
		c.bodyB->velocity += impulse * c.inverseMassB;
		c.bodyB->angularVelocity += c.inverseInertiaB * glm::cross(p.rB, impulse);
	}
}

void ContactSolver::Prepare(const std::vector<Result>& manifolds, float dt) {
	constraints.clear();
	constraints.reserve(manifolds.size());

	for (const Result& manifold : manifolds) {
		Constraint c;
		c.bodyA = manifold.a->GetRigidBody();
		c.bodyB = manifold.b->GetRigidBody();

		//static bodies and plain colliders keep 0 inverse mass, i.e. infinitely heavy
		if (IsDynamic(c.bodyA)) {
			c.inverseMassA = c.bodyA->GetInverseMass();
			c.inverseInertiaA = c.bodyA->GetInverseInertiaWorld();
		}
		if (IsDynamic(c.bodyB)) {
			c.inverseMassB = c.bodyB->GetInverseMass();
			c.inverseInertiaB = c.bodyB->GetInverseInertiaWorld();
		}
		if (c.inverseMassA + c.inverseMassB == 0.0f) continue;

		c.normal = manifold.normal;
		ComputeBasis(c.normal, c.tangent[0], c.tangent[1]);
		c.friction = std::sqrt(manifold.a->friction * manifold.b->friction);
		float restitution = std::max(manifold.a->restitution, manifold.b->restitution);

		for (int i = 0; i < manifold.contactCount; ++i) {
			const ContactPoint& contact = manifold.contacts[i];
			Point& p = c.points[c.pointCount++];

			if (c.bodyA) p.rA = contact.position - c.bodyA->GetCenterMass();
			if (c.bodyB) p.rB = contact.position - c.bodyB->GetCenterMass();

			p.normalMass = EffectiveMass(c, p.rA, p.rB, c.normal);
			p.tangentMass[0] = EffectiveMass(c, p.rA, p.rB, c.tangent[0]);
			p.tangentMass[1] = EffectiveMass(c, p.rA, p.rB, c.tangent[1]);

			if (contact.penetration < 0.0f) {
				//speculative: the point is still a small gap away. Allow closing the gap this step
				//but no further, otherwise the body would be held up by a point it isn't touching
				p.bias = contact.penetration / dt;
				continue;
			}

			//push apart a fraction of the penetration each step, as extra separating velocity
			float positionBias = baumgarte / dt * std::max(contact.penetration - slop, 0.0f);

			//bounce: reflect part of the approach velocity measured before solving
			float approachVelocity = glm::dot(VelocityAt(c.bodyB, p.rB) - VelocityAt(c.bodyA, p.rA), c.normal);
			float restitutionBias = approachVelocity < -restitutionThreshold ? -restitution * approachVelocity : 0.0f;

			p.bias = std::max(positionBias, restitutionBias);
		}
		constraints.push_back(c);
	}
}

void ContactSolver::SolveVelocities() {
	for (Constraint& c : constraints) {
		for (int i = 0; i < c.pointCount; ++i) {
			Point& p = c.points[i];

			//friction first, it's limited by the normal impulse from the previous iteration
			for (int t = 0; t < 2; ++t) {
				glm::vec3 relativeVelocity = VelocityAt(c.bodyB, p.rB) - VelocityAt(c.bodyA, p.rA);
				float lambda = -glm::dot(relativeVelocity, c.tangent[t]) * p.tangentMass[t];

				//coulomb friction: total friction can't exceed friction * total normal force
				float maxFriction = c.friction * p.normalImpulse;
				float oldImpulse = p.tangentImpulse[t];
				p.tangentImpulse[t] = glm::clamp(oldImpulse + lambda, -maxFriction, maxFriction);
				lambda = p.tangentImpulse[t] - oldImpulse;

				ApplyImpulse(c, p, lambda * c.tangent[t]);
			}

			glm::vec3 relativeVelocity = VelocityAt(c.bodyB, p.rB) - VelocityAt(c.bodyA, p.rA);
			float normalVelocity = glm::dot(relativeVelocity, c.normal);
			float lambda = p.normalMass * (-normalVelocity + p.bias);

			//clamp the accumulated impulse, not this iteration's: a single iteration may pull back
			//an earlier overshoot, but the contact as a whole can only push
			float oldImpulse = p.normalImpulse;
			p.normalImpulse = std::max(oldImpulse + lambda, 0.0f);
			lambda = p.normalImpulse - oldImpulse;

			ApplyImpulse(c, p, lambda * c.normal);
		}
	}
}
