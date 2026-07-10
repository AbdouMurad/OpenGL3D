#include "GameObject.h"


Transform::Transform(glm::vec3 Pos, glm::vec3 Size, glm::vec3 Rot)
	:	position(Pos), size(Size), rotation(glm::quat(glm::radians(Rot))) {}
Transform::Transform(glm::vec3 Pos)
	: position(Pos) {}

glm::vec3 Transform::getPosition() {
	return position;
}
glm::vec3 Transform::getSize() {
	return size;
}
glm::vec3 Transform::getRotation() {
	return glm::degrees(glm::eulerAngles(rotation));
}

void Transform::setPosition(glm::vec3 position) {
	Transform::position = position;
}
void Transform::setSize(glm::vec3 size) {
	Transform::size = size;
}
void Transform::setRotation(glm::vec3 rotation) {
	Transform::rotation = glm::quat(glm::radians(rotation));
}

void Transform::translate(glm::vec3 delta) {
	position += delta;
}
void Transform::rotate(glm::vec3 rotation) {
	Transform::rotation = glm::normalize(glm::quat(glm::radians(rotation)) * Transform::rotation);
}
void Transform::scale(float scale) {
	size *= scale;
}

std::ostream& operator<<(std::ostream& os, const Transform& transform) {
	glm::vec3 EulerRotation = glm::degrees(glm::eulerAngles(transform.rotation));
	os << "Transform: {\n";
	os << "	Position: { x: " << transform.position.x << ", y: " << transform.position.y << ", z: " << transform.position.z << "}\n";
	os << "	Rotation: { x: " << EulerRotation.x << ", y: " << EulerRotation.y << ", z: " << EulerRotation.z << "}\n";
	os << "	Size: { x: " << transform.size.x << ", y: " << transform.size.y << ", z: " << transform.size.z << "}\n";
	os << "}\n";
	return os;
}