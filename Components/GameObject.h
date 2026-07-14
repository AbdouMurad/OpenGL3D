#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp> 
#include <cmath>
#include <iostream>
#include "Core/Handlers.h"

class Transform {
private:
	glm::vec3 position = glm::vec3(0,0,0);
	glm::quat rotation = glm::quat(1,0,0,0);
	glm::vec3 size = glm::vec3(1, 1, 1);
public:
	Transform(glm::vec3 position, glm::vec3 size, glm::vec3 rotation);
	Transform(glm::vec3 position);
	Transform() = default;

	glm::vec3 getPosition();
	glm::vec3 getSize();
	glm::vec3 getRotation();
	glm::quat getQuat();

	void setPosition(glm::vec3 pos);
	void setSize(glm::vec3 size);
	void setRotation(glm::vec3 rotation);
	void setRotation(glm::quat rotation);

	void translate(glm::vec3 delta);
	void rotate(glm::vec3 rotation);
	void scale(float scale);

	friend std::ostream& operator<<(std::ostream& os, const Transform& transform);
};

class GameObject {
public: 
	Transform transform;
	ModelHandle model;

};