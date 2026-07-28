#include "Shape.h"

Sphere::Sphere(float radius)
	:	radius(radius) {}

SHAPE Sphere::GetType() const {
	return SHAPE::Sphere;
}

Box::Box(const glm::vec3& halfSize)
	:	halfExtent(halfSize) {}
SHAPE Box::GetType() const {
	return SHAPE::Box;
}

Capsule::Capsule(float radius)
	:	radius(radius) {}
Capsule::Capsule(const glm::vec3& point1, const glm::vec3& point2)
	:	point1(point1), point2(point2) {}
Capsule::Capsule(const glm::vec3& point1, const glm::vec3& point2, float radius)
	:	point1(point1), point2(point2), radius(radius) {}
SHAPE Capsule::GetType() const {
	return SHAPE::Capsule;
}