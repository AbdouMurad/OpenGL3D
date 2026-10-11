#pragma once

#include <glm/glm.hpp>

class ColliderComponent;

enum class SHAPE {
	Sphere = 0, Box, Capsule, SIZE
};


struct ColliderShape {
	virtual ~ColliderShape() = default;
	virtual SHAPE GetType() const = 0;
};

struct Sphere : public ColliderShape {
	Sphere() = default;
	Sphere(float radius);

	float GetRadius() const { return radius; }

	SHAPE GetType() const override;
private:
	//only editable through ColliderComponent::SetRadius -> keeps rigidbody inertia in sync
	friend class ColliderComponent;
	float radius = 0.5f;
};

struct Box : public ColliderShape {
	Box() = default;
	Box(const glm::vec3& halfSize);

	const glm::vec3& GetHalfExtent() const { return halfExtent; }

	SHAPE GetType() const override;
private:
	//only editable through ColliderComponent::SetHalfExtent -> keeps rigidbody inertia in sync
	friend class ColliderComponent;
	glm::vec3 halfExtent = glm::vec3(0.5);
};

struct Capsule : public ColliderShape {
	Capsule() = default;
	Capsule(float radius);
	Capsule(const glm::vec3& point1, const glm::vec3& point2);
	Capsule(const glm::vec3& point1, const glm::vec3& point2, float radius);

	glm::vec3 point1 = glm::vec3(0,1,0);
	glm::vec3 point2 = glm::vec3(0,-1,0);
	float radius = 1.0f;

	SHAPE GetType() const override;
	
};