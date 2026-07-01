#pragma once

#include <string>

#include "Buffer.h"
#include "Texture.h" 

class Mesh {
public:
	glm::vec3 position; //temp -> move to model class

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	VAO VAO;
	VBO vbo;
	EBO ebo;

	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture>& textures);

};