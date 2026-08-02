#include "RigidBody.h"

RigidBodyComponent::RigidBodyComponent(float mass, bool isStatic)
	: mass(mass), inverseMass(1.0f / mass), isStatic(isStatic) {
}
RigidBodyComponent::RigidBodyComponent(float mass)
	: mass(mass), inverseMass(1.0f / mass) {}

void RigidBodyComponent::AddForce(const glm::vec3& force) {
	accumulatedForce += force;
}
void RigidBodyComponent::AddTorque(const glm::vec3& torque) {
	accumulatedTorque += torque;
}
void RigidBodyComponent::Move(const glm::vec3& delta) {
	owner->GetComponent<TransformComponent>()->Translate(delta);
}

void RigidBodyComponent::AddImpulse(const glm::vec3& impulse) {
	pendingImpulse += impulse;
}
void RigidBodyComponent::AddAngularImpulse(const glm::vec3& angularImpulse) {
	pendingAngularImpulse += angularImpulse;
}
const glm::vec3& RigidBodyComponent::GetVelocity() const {
	return velocity;
}
const glm::vec3& RigidBodyComponent::GetAngularVelocity() const {
	return angularVelocity;
}
void RigidBodyComponent::SetVelocity(const glm::vec3& velocity) {
	RigidBodyComponent::velocity = velocity;
}
void RigidBodyComponent::SetAngularVelocity(const glm::vec3& angularVelocity) {
	RigidBodyComponent::angularVelocity = angularVelocity;
}
float RigidBodyComponent::GetMass() const {
	return mass;
}
float RigidBodyComponent::GetInverseMass() const {
	return inverseMass;
}
void RigidBodyComponent::SetMass(float m) {
	mass = m;
	inverseMass = 1.0f / m;
}
bool RigidBodyComponent::IsStatic() const {
	return isStatic;
}
bool RigidBodyComponent::UseGravity() const {
	return useGravity;
}
void RigidBodyComponent::ToggleStatic() {
	isStatic = !isStatic;
}
void RigidBodyComponent::ToggleGravity() {
	useGravity = !useGravity;
}

