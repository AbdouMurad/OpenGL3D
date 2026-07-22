#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>

#include "Core/gl_core.h"
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>
#include<glm/geometric.hpp>

#include "Core/Input.h"
#include "GameObject.h"

class CameraComponent : public Component {
public:
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 cameraMatrix = glm::mat4(1.0f);
	
	int width;
	int height;

	float FOVdeg = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;

	CameraComponent(int width, int height);

	void setViewport(int width, int height);
	void updateMatrix();
	
	glm::vec3 getOrientation();
	glm::vec3 getPosition();

};