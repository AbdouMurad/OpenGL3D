#include "Camera.h"

CameraComponent::CameraComponent(int width, int height) {
	CameraComponent::width = width;
	CameraComponent::height = height;
}

void CameraComponent::updateMatrix(float FOVdeg, float nearPlane, float farPlane) {
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	glm::vec3 position = owner->GetComponent<TransformComponent>()->getPosition();

	view = glm::lookAt(position, position + getOrientation(), Up);
	
	projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);
	cameraMatrix = projection * view;
}

void CameraComponent::setViewport(int width, int height) {
	CameraComponent::width = width;
	CameraComponent::height = height;
}

glm::vec3 CameraComponent::getOrientation() {
	return glm::normalize(
		owner->GetComponent<TransformComponent>()->getQuat() * glm::vec3(0, 0, -1)
	);
}
