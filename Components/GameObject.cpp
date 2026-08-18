#include "GameObject.h"

glm::mat4 Transform::GetMatrix() const {
	glm::mat4 mat(1.0f);

	mat = glm::translate(mat, position);
	mat *= glm::mat4(rotation);
	mat = glm::scale(mat, size);

	return mat;
}

TransformComponent::TransformComponent(glm::vec3 Pos, glm::vec3 Size, glm::vec3 Rot) {
	local.position = Pos;
	local.size = Size;
	local.rotation = Rot;
}
TransformComponent::TransformComponent(glm::vec3 Pos) {
	local.position = Pos;
}


glm::vec3 TransformComponent::GetPosition() const {
	glm::mat4 world = GetMatrix();
	return glm::vec3(world[3]);
}
glm::vec3 TransformComponent::GetLocalPosition() const {
	return local.position;
}


glm::vec3 TransformComponent::GetSize() const {
	glm::mat4 world = GetMatrix();

	return glm::vec3(
		glm::length(glm::vec3(world[0])),
		glm::length(glm::vec3(world[1])),
		glm::length(glm::vec3(world[2]))
	);
}
glm::vec3 TransformComponent::GetLocalSize() const {
	return local.size;
}

glm::vec3 TransformComponent::GetRotation() const {
	return glm::degrees(glm::eulerAngles(GetQuat()));
}
glm::vec3 TransformComponent::GetLocalRotation() const {
	return glm::degrees(glm::eulerAngles(local.rotation));
}

glm::quat TransformComponent::GetQuat() const{
	glm::mat4 world = GetMatrix();
	glm::vec3 scale = {
		glm::length(glm::vec3(world[0])),
		glm::length(glm::vec3(world[1])),
		glm::length(glm::vec3(world[2]))
	};
	glm::mat3 rot(
		glm::vec3(world[0]) / scale.x,
		glm::vec3(world[1]) / scale.y,
		glm::vec3(world[2]) / scale.z
	);
	return glm::quat_cast(rot);
}
glm::quat TransformComponent::GetLocalQuat() const {
	return local.rotation;
}

glm::mat4 TransformComponent::GetLocalMatrix() const
{
	return local.GetMatrix();
}
glm::mat4 TransformComponent::GetMatrix() const
{
	glm::mat4 parentMatrix = parent ? parent->GetMatrix() : glm::mat4(1.0f);
	return parentMatrix * GetLocalMatrix();
}

void TransformComponent::SetPosition(const glm::vec3& position) {
	local.position = position;
}
void TransformComponent::SetSize(const glm::vec3& size) {
	local.size = size;
}

void TransformComponent::SetRotation(const glm::vec3& rotation) {
	local.rotation = glm::quat(glm::radians(rotation));
}
void TransformComponent::SetRotation(const glm::quat& rotation) {
	local.rotation = rotation;
}

void TransformComponent::Translate(const glm::vec3& delta) {
	local.position += delta;
}
void TransformComponent::Rotate(const glm::vec3& delta) {
	glm::quat deltaQuat =
		glm::angleAxis(glm::radians(delta.x), glm::vec3(1, 0, 0)) *
		glm::angleAxis(glm::radians(delta.y), glm::vec3(0, 1, 0)) *
		glm::angleAxis(glm::radians(delta.z), glm::vec3(0, 0, 1));

	local.rotation = glm::normalize(local.rotation * deltaQuat);
}
void TransformComponent::Rotate(const glm::quat& delta) {
	local.rotation = glm::normalize(local.rotation * delta);
}
void TransformComponent::Scale(float scale) {
	local.size *= scale;
}

glm::vec3 TransformComponent::Forward() const {
	return glm::normalize(local.rotation * glm::vec3(0, 0, -1));
}
glm::vec3 TransformComponent::Right() const {
	return glm::normalize(local.rotation * glm::vec3(1, 0, 0));
}
glm::vec3 TransformComponent::Up() const {
	return glm::normalize(local.rotation * glm::vec3(0, 1, 0));
}

std::ostream& operator<<(std::ostream& os, const TransformComponent& transform) {
	glm::vec3 EulerRotation = glm::degrees(glm::eulerAngles(transform.local.rotation));
	glm::vec3 worldPos = transform.GetPosition();
	os << "TransformComponent: {\n";
	os << "	Position: { x: " << transform.local.position.x << ", y: " << transform.local.position.y << ", z: " << transform.local.position.z << "}\n";
	os << "	Rotation: { x: " << EulerRotation.x << ", y: " << EulerRotation.y << ", z: " << EulerRotation.z << "}\n";
	os << "	Size: { x: " << transform.local.size.x << ", y: " << transform.local.size.y << ", z: " << transform.local.size.z << "}\n";
	os << "	World Position: { x: " << worldPos.x << ", y: " << worldPos.y << ", z: " << worldPos.z << "}\n";
	os << "}\n";
	return os;
}



//-------------------Mesh Renderer-----------------
MeshRenderer::MeshRenderer(ModelHandle m) : modelID(m) {};
