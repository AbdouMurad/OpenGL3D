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