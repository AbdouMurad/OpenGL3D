#include "GameObject.h"

Transform::Transform()
	: position({ 0.0f, 0.0f, 0.0f }), size({ 1.0f, 1.0f, 1.0f }), rotation({ 0.0f, 0.0f, 0.0f }) {}
Transform::Transform(glm::vec3 Pos, glm::vec3 Size, glm::vec3 Rot)
	:	position(Pos), size(Size), rotation(Rot) {}
Transform::Transform(glm::vec3 Pos)
	: position(Pos), size({ 1.0f, 1.0f, 1.0f }), rotation({ 0.0f, 0.0f, 0.0f }) {}

glm::vec3 Transform::getPosition() {
	return position;
}
glm::vec3 Transform::getSize() {
	return size;
}
glm::vec3 Transform::getRotation() {
	return rotation;
}

void Transform::setPosition(glm::vec3 position) {
	Transform::position = position;
}
void Transform::setSize(glm::vec3 size) {
	Transform::size = size;
}
void Transform::setRotation(glm::vec3 rotation) {
	Transform::rotation = glm::vec3(std::fmod(rotation.x, 360), std::fmod(rotation.y, 360), std::fmod(rotation.z, 360));

}

void Transform::translate(glm::vec3 delta) {
	position += delta;
}
void Transform::rotate(glm::vec3 rotation) {
	setRotation(rotation + Transform::rotation);
}
void Transform::scale(float scale) {
	size *= scale;
}

std::ostream& operator<<(std::ostream& os, const Transform& transform) {
	os << "Transform: {\n";
	os << "	Position: { x: " << transform.position.x << ", y: " << transform.position.y << ", z: " << transform.position.z << "}\n";
	os << "	Rotation: { x: " << transform.rotation.x << ", y: " << transform.rotation.y << ", z: " << transform.rotation.z << "}\n";
	os << "	Size: { x: " << transform.size.x << ", y: " << transform.size.y << ", z: " << transform.size.z << "}\n";
	os << "}\n";
	return os;
}