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

void RigidBodyComponent::SetLocalInertiaTensor(const Box* box) {
	localInertiaTensor = glm::mat3(0);
	glm::vec3 dimensions = box->halfExtent * 2.0f;
	localInertiaTensor[0][0] = mass * (dimensions.y * dimensions.y + dimensions.z * dimensions.z) / 12.0f; //Ix
	localInertiaTensor[1][1] = mass * (dimensions.x * dimensions.x + dimensions.z * dimensions.z) / 12.0f; //Iy
	localInertiaTensor[2][2] = mass * (dimensions.x * dimensions.x + dimensions.y * dimensions.y) / 12.0f; //Iz

	inverseLocalInertiaTensor = glm::mat3(0);
	inverseLocalInertiaTensor[0][0] = 1.0f / localInertiaTensor[0][0];
	inverseLocalInertiaTensor[1][1] = 1.0f / localInertiaTensor[0][0];
	inverseLocalInertiaTensor[2][2] = 1.0f / localInertiaTensor[0][0];

}
void RigidBodyComponent::SetLocalInertiaTensor(const Sphere* sphere) {
	localInertiaTensor = glm::mat3(0);
	float inertia = mass * sphere->radius * sphere->radius * 2.0f / 3.0f;
	localInertiaTensor[0][0] = inertia;
	localInertiaTensor[1][1] = inertia;
	localInertiaTensor[2][2] = inertia;

	inverseLocalInertiaTensor = glm::mat3(0);
	inverseLocalInertiaTensor[0][0] = 1.0f / localInertiaTensor[0][0];
	inverseLocalInertiaTensor[1][1] = 1.0f / localInertiaTensor[0][0];
	inverseLocalInertiaTensor[2][2] = 1.0f / localInertiaTensor[0][0];
}

const glm::mat3& RigidBodyComponent::GetLocalInertiaTensor() const {
	return localInertiaTensor;
}
const glm::mat3& RigidBodyComponent::GetInverseLocalInertiaTensor() const {
	return inverseLocalInertiaTensor;
}