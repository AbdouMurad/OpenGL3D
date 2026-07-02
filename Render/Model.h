#pragma once

#include <string>

#include "Buffer.h"
#include "Texture.h" 

class Mesh {
public:
	glm::vec3 position; //temp -> move to model class

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	VAO VAO;
	EBO ebo;
	VBO vbo;

	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices);

};