#include "GameObject.h"
#include "Render/Renderer.h"

TransformComponent::TransformComponent(glm::vec3 Pos, glm::vec3 Size, glm::vec3 Rot)
	:	position(Pos), size(Size), rotation(glm::quat(glm::radians(Rot))) {}
TransformComponent::TransformComponent(glm::vec3 Pos)
	: position(Pos) {}

glm::vec3 TransformComponent::getPosition() {
	return position;
}
glm::vec3 TransformComponent::getSize() {
	return size;
}
glm::vec3 TransformComponent::getRotation() {
	return glm::degrees(glm::eulerAngles(rotation));
}
glm::quat TransformComponent::getQuat() {
	return rotation;
}

glm::mat4 TransformComponent::getMatrix() const {
	glm::mat4 mat = glm::translate(glm::mat4(1.0f), position);
	mat *= glm::mat4(rotation);
	mat *= glm::scale(mat, size);

	return mat;
}

void TransformComponent::setPosition(glm::vec3 position) {
	TransformComponent::position = position;
}
void TransformComponent::setSize(glm::vec3 size) {
	TransformComponent::size = size;
}

void TransformComponent::setRotation(glm::vec3 rotation) {
	TransformComponent::rotation = glm::quat(glm::radians(rotation));
}
void TransformComponent::setRotation(glm::quat rotation) {
	TransformComponent::rotation = rotation;
}

void TransformComponent::translate(glm::vec3 delta) {
	position += delta;
}
void TransformComponent::rotate(glm::vec3 delta)
{
	glm::quat deltaQuat =
		glm::angleAxis(glm::radians(delta.x), glm::vec3(1, 0, 0)) *
		glm::angleAxis(glm::radians(delta.y), glm::vec3(0, 1, 0)) *
		glm::angleAxis(glm::radians(delta.z), glm::vec3(0, 0, 1));

	rotation = glm::normalize(rotation * deltaQuat);
}
void TransformComponent::scale(float scale) {
	size *= scale;
}

std::ostream& operator<<(std::ostream& os, const TransformComponent& transform) {
	glm::vec3 EulerRotation = glm::degrees(glm::eulerAngles(transform.rotation));
	os << "TransformComponent: {\n";
	os << "	Position: { x: " << transform.position.x << ", y: " << transform.position.y << ", z: " << transform.position.z << "}\n";
	os << "	Rotation: { x: " << EulerRotation.x << ", y: " << EulerRotation.y << ", z: " << EulerRotation.z << "}\n";
	os << "	Size: { x: " << transform.size.x << ", y: " << transform.size.y << ", z: " << transform.size.z << "}\n";
	os << "}\n";
	return os;
}


//-------------------Mesh Renderer-----------------
MeshRenderer::MeshRenderer(ModelHandle m) : modelID(m) {};

void MeshRenderer::Render(Renderer& renderer) {
	TransformComponent* transform = owner->GetComponent<TransformComponent>();
	renderer.Draw(AssetManager::Get().GetModel(modelID), *transform);
}