#include "Camera.h"

Camera::Camera(int width, int height, glm::vec3 position) {
	Camera::width = width;
	Camera::height = height;
	transform.setPosition(position);
}

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane) { 
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	glm::vec3 position = transform.getPosition();
	

	view = glm::lookAt(position, position + getOrientation(), Up);
	
	projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);
	cameraMatrix = projection * view;
}

void Camera::setViewport(int width, int height) {
	Camera::width = width;
	Camera::height = height;
}

glm::vec3 Camera::getOrientation() {
	return glm::normalize(
		transform.getQuat() * glm::vec3(0, 0, -1)
	);
}
