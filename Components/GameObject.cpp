#include "GameObject.h"
#include "Render/Renderer.h"

TransformComponent::TransformComponent(glm::vec3 Pos, glm::vec3 Size, glm::vec3 Rot)
	:	position(Pos), size(Size), rotation(glm::quat(glm::radians(Rot))) {}
TransformComponent::TransformComponent(glm::vec3 Pos)
	: position(Pos) {}


glm::vec3 TransformComponent::getWorldPosition() const {
	glm::mat4 world = getWorldMatrix();
	return glm::vec3(world[3]);
}
glm::vec3 TransformComponent::getPosition() const {
	return position;
}


glm::vec3 TransformComponent::getWorldSize() const {
	glm::mat4 world = getWorldMatrix();

	return glm::vec3(
		glm::length(glm::vec3(world[0])),
		glm::length(glm::vec3(world[1])),
		glm::length(glm::vec3(world[2]))
	);
}
glm::vec3 TransformComponent::getSize() const {
	return size;
}

glm::vec3 TransformComponent::getWorldRotation() const {
	return glm::degrees(glm::eulerAngles(getWorldQuat()));
}
glm::vec3 TransformComponent::getRotation() const {
	return glm::degrees(glm::eulerAngles(rotation));
}

glm::quat TransformComponent::getWorldQuat() const{
	glm::mat4 world = getWorldMatrix();
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
glm::quat TransformComponent::getQuat() const {
	return rotation;
}

glm::mat4 TransformComponent::getLocalMatrix() const
{
	glm::mat4 mat(1.0f);

	mat = glm::translate(mat, position);
	mat *= glm::mat4(rotation);
	mat = glm::scale(mat, size);

	return mat;
}
glm::mat4 TransformComponent::getWorldMatrix() const
{
	glm::mat4 parentMatrix(1.0f);
	if (parent)
	{
		glm::mat4 parentWorld = parent->getWorldMatrix();

		if (!inheritScale)
		{
			parentWorld[0] = glm::normalize(parentWorld[0]);
			parentWorld[1] = glm::normalize(parentWorld[1]);
			parentWorld[2] = glm::normalize(parentWorld[2]);
		}
		parentMatrix = parentWorld;
	}
	return parentMatrix * getLocalMatrix();
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
	glm::vec3 worldPos = transform.getWorldPosition();
	os << "TransformComponent: {\n";
	os << "	Position: { x: " << transform.position.x << ", y: " << transform.position.y << ", z: " << transform.position.z << "}\n";
	os << "	Rotation: { x: " << EulerRotation.x << ", y: " << EulerRotation.y << ", z: " << EulerRotation.z << "}\n";
	os << "	Size: { x: " << transform.size.x << ", y: " << transform.size.y << ", z: " << transform.size.z << "}\n";
	os << "	World Position: { x: " << worldPos.x << ", y: " << worldPos.y << ", z: " << worldPos.z << "}\n";
	os << "}\n";
	return os;
}


//------------------Game Object-----------------
void GameObject::Render(Renderer& renderer) {
	MeshRenderer* component = GetComponent<MeshRenderer>();
	if (component) component->Render(renderer);
}


//-------------------Mesh Renderer-----------------
MeshRenderer::MeshRenderer(ModelHandle m) : modelID(m) {};

void MeshRenderer::Render(Renderer& renderer) {
	renderer.Draw(AssetManager::Get().GetModel(modelID), *(owner->GetComponent<TransformComponent>()));
}