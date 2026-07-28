#pragma once

#include "Shape.h"
#include "Components/GameObject.h"
#include <memory>

class RigidBodyComponent;
class ColliderComponent : public Component {
public:
	ColliderComponent(SHAPE shape);

	std::unique_ptr<ColliderShape> shape = nullptr;
	Transform localOffset;

	glm::mat4 GetMatrix() const;

	bool isTrigger = false;

	RigidBodyComponent* GetRigidBody();
};