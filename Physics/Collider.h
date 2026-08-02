#pragma once

#include "Shape.h"
#include "Components/GameObject.h"
#include <memory>

struct AABB {
	glm::vec3 max = glm::vec3(0);
	glm::vec3 min = glm::vec3(0);
};

class RigidBodyComponent;
class ColliderComponent : public Component {
public:
	ColliderComponent(SHAPE shape);

	AABB aabb;
	std::unique_ptr<ColliderShape> shape = nullptr;
	Transform localOffset;

	glm::mat4 GetMatrix() const;

	bool isTrigger = false;

	RigidBodyComponent* GetRigidBody();
};