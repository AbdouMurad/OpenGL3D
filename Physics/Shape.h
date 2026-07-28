#pragma once

#include <glm/glm.hpp>

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

	float radius = 1.0f;

	SHAPE GetType() const override;
};

struct Box : public ColliderShape {
	Box() = default;
	Box(const glm::vec3& halfSize);

	glm::vec3 halfExtent = glm::vec3(1);

	SHAPE GetType() const override;
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