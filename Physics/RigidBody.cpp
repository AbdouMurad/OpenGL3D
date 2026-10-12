#include "RigidBody.h"
#include "Collider.h"

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
const glm::vec3 RigidBodyComponent::GetCenterMass() const {
	return owner->GetComponent<TransformComponent>()->GetPosition();
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
	RecalculateInertia();
}
float RigidBodyComponent::GetRestitution() const {
	return restitution;
}
void RigidBodyComponent::SetRestitution(float r) {
	restitution = glm::clamp(r, 0.0f, 1.0f); //above 1 would add energy on every bounce
}
float RigidBodyComponent::GetFriction() const {
	return friction;
}
void RigidBodyComponent::SetFriction(float f) {
	friction = glm::max(f, 0.0f); //negative friction would push sliding bodies faster
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

void RigidBodyComponent::Start() {
	RecalculateInertia(); //collider may have been added before this rigidbody
}
void RigidBodyComponent::RecalculateInertia() {
	localInertiaTensor = glm::mat3(0);
	inverseLocalInertiaTensor = glm::mat3(0); //no solid collider -> body can't rotate
	if (!owner) return; //not attached yet, Start() will recalculate
	if (IsRotationFrozen()) return; //never rotates -> no tensor needed (unlocking recalculates)

	//uses the first solid collider (triggers have no mass)
	for (ColliderComponent* collider : owner->GetComponents<ColliderComponent>()) {
		if (collider->IsTrigger()) continue;
		switch (collider->shape->GetType()) {
		case (SHAPE::Box):
			SetLocalInertiaTensor(static_cast<Box*>(collider->shape.get()));
			return;
		case (SHAPE::Sphere):
			SetLocalInertiaTensor(static_cast<Sphere*>(collider->shape.get()));
			return;
		default:
			break; //TODO: Capsule
		}
	}
}
void RigidBodyComponent::SetLocalInertiaTensor(const Box* box) {
	localInertiaTensor = glm::mat3(0);
	glm::vec3 dimensions = box->GetHalfExtent() * 2.0f;
	localInertiaTensor[0][0] = mass * (dimensions.y * dimensions.y + dimensions.z * dimensions.z) / 12.0f; //Ix
	localInertiaTensor[1][1] = mass * (dimensions.x * dimensions.x + dimensions.z * dimensions.z) / 12.0f; //Iy
	localInertiaTensor[2][2] = mass * (dimensions.x * dimensions.x + dimensions.y * dimensions.y) / 12.0f; //Iz

	inverseLocalInertiaTensor = glm::mat3(0);
	inverseLocalInertiaTensor[0][0] = 1.0f / localInertiaTensor[0][0];
	inverseLocalInertiaTensor[1][1] = 1.0f / localInertiaTensor[1][1];
	inverseLocalInertiaTensor[2][2] = 1.0f / localInertiaTensor[2][2];
}
void RigidBodyComponent::SetLocalInertiaTensor(const Sphere* sphere) {
	localInertiaTensor = glm::mat3(0);
    // Solid sphere inertia: I = 2/5 m r^2
    float inertia = mass * sphere->GetRadius() * sphere->GetRadius() * 2.0f / 5.0f;
	localInertiaTensor[0][0] = inertia;
	localInertiaTensor[1][1] = inertia;
	localInertiaTensor[2][2] = inertia;

	inverseLocalInertiaTensor = glm::mat3(0);
	inverseLocalInertiaTensor[0][0] = 1.0f / localInertiaTensor[0][0];
	inverseLocalInertiaTensor[1][1] = 1.0f / localInertiaTensor[1][1];
	inverseLocalInertiaTensor[2][2] = 1.0f / localInertiaTensor[2][2];
}

const glm::mat3& RigidBodyComponent::GetLocalInertiaTensor() const {
	return localInertiaTensor;
}
const glm::mat3& RigidBodyComponent::GetInverseLocalInertiaTensor() const {
	return inverseLocalInertiaTensor;
}
glm::mat3 RigidBodyComponent::GetInverseInertiaWorld() const {
	if (IsRotationFrozen()) return glm::mat3(0.0f); //skips the world matrix rebuild in GetQuat()
	glm::mat3 R = glm::mat3_cast(owner->GetComponent<TransformComponent>()->GetQuat());
	glm::mat3 inverseInertiaWorld = R * inverseLocalInertiaTensor * glm::transpose(R);

	//locks are on world axes (same as ApplyConstraint) -> zero that row + column
	//so a locked axis neither takes torque in nor gives angular velocity out
	glm::mat3 mask(1.0f);
	for (int i = 0; i < 3; ++i) {
		if (IsRotationLocked(i)) mask[i][i] = 0.0f;
	}
	return mask * inverseInertiaWorld * mask;
}
//constraints decide whether an inertia tensor is needed -> recalculate on change (e.g. unlocking a frozen body)
void RigidBodyComponent::SetConstraint(RigidBodyConstraints c) {
	constraints = c;
	RecalculateInertia();
}
void RigidBodyComponent::AddConstraint(RigidBodyConstraints c) {
	constraints = constraints | c;
	RecalculateInertia();
}
bool RigidBodyComponent::IsRotationFrozen() const {
	constexpr uint8_t all = static_cast<uint8_t>(RigidBodyConstraints::FreezeRotation);
	return (static_cast<uint8_t>(constraints) & all) == all;
}
bool RigidBodyComponent::IsRotationLocked(int axis) const {
	return (static_cast<uint8_t>(constraints) & (1 << axis)) != 0;
}
void RigidBodyComponent::ApplyConstraint() {
	if (IsRotationLocked(0)) angularVelocity.x = 0;
	if (IsRotationLocked(1)) angularVelocity.y = 0;
	if (IsRotationLocked(2)) angularVelocity.z = 0;
}