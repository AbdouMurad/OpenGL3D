#pragma once

#include<glm/glm.hpp>
#include<cmath>
#include <iostream>

class Transform {
private:
	glm::vec3 position;
	glm::vec3 size;
	glm::vec3 rotation;
public:
	Transform(glm::vec3 position, glm::vec3 size, glm::vec3 rotation);
	Transform(glm::vec3 position);
	Transform();

	glm::vec3 getPosition();
	glm::vec3 getSize();
	glm::vec3 getRotation();

	void setPosition(glm::vec3 pos);
	void setSize(glm::vec3 size);
	void setRotation(glm::vec3 rotation);

	void translate(glm::vec3 delta);
	void rotate(glm::vec3 rotation);
	void scale(float scale);

	friend std::ostream& operator<<(std::ostream& os, const Transform& transform);
};

