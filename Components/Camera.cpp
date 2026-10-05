#include "Camera.h"

CameraComponent::CameraComponent(int width, int height) {
	CameraComponent::width = width;
	CameraComponent::height = height;
}

void CameraComponent::UpdateMatrix() {
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	TransformComponent* transform = owner->GetComponent<TransformComponent>();
	

	//world space up: Up() only uses the local rotation, which rolls the view when the camera has a rotated parent
	glm::vec3 up = transform->GetQuat() * glm::vec3(0, 1, 0);
	view = glm::lookAt(transform->GetPosition(), transform->GetPosition() + GetOrientation(), up);
	
	projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);
	cameraMatrix = projection * view;
}

void CameraComponent::SetViewport(int width, int height) {
	CameraComponent::width = width;
	CameraComponent::height = height;
}

glm::vec3 CameraComponent::GetOrientation() {
	return glm::normalize(
		owner->GetComponent<TransformComponent>()->GetQuat() * glm::vec3(0, 0, -1)
	);
}
glm::vec3 CameraComponent::GetPosition() {
	return owner->GetComponent<TransformComponent>()->GetPosition();
}
