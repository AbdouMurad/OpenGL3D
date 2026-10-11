#include "Collider.h"
#include "RigidBody.h"
glm::mat4 ColliderComponent::GetMatrix() const {
	TransformComponent* parentTransform = owner->GetComponent<TransformComponent>();
	return parentTransform->GetMatrix() * localOffset.GetMatrix();
}
//maybe get rid of
RigidBodyComponent* ColliderComponent::GetRigidBody() {
	return owner->GetComponent<RigidBodyComponent>();
}
ColliderComponent::ColliderComponent(SHAPE s) {
	switch (s) {
	case (SHAPE::Box):
		shape = std::make_unique<Box>();
		break;
	case (SHAPE::Sphere):
		shape = std::make_unique<Sphere>();
		break;
	case (SHAPE::Capsule):
		shape = std::make_unique<Capsule>();
		break;
	}
}
void ColliderComponent::Start() {
	NotifyRigidBody();
}

bool ColliderComponent::IsTrigger() const {
	return isTrigger;
}
void ColliderComponent::SetTrigger(bool trigger) {
	isTrigger = trigger;
	NotifyRigidBody(); //triggers don't count towards inertia
}
void ColliderComponent::SetRadius(float radius) {
	if (shape->GetType() != SHAPE::Sphere) {
		std::cout << "SetRadius called on a non-sphere collider\n";
		return;
	}
	static_cast<Sphere*>(shape.get())->radius = radius;
	NotifyRigidBody();
}
void ColliderComponent::SetHalfExtent(const glm::vec3& halfExtent) {
	if (shape->GetType() != SHAPE::Box) {
		std::cout << "SetHalfExtent called on a non-box collider\n";
		return;
	}
	static_cast<Box*>(shape.get())->halfExtent = halfExtent;
	NotifyRigidBody();
}

//inertia depends on the collider's shape -> rigidbody must recompute whenever it changes
void ColliderComponent::NotifyRigidBody() {
	if (!owner) return; //not attached to an object yet, Start() will notify
	if (RigidBodyComponent* rb = GetRigidBody()) rb->RecalculateInertia();
}