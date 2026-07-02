#pragma once

#include "GPU/Buffer.h"
#include "Components/GameObject.h"

class Mesh {
public:
	Transform transform;

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	VAO VAO;
	EBO ebo;
	VBO vbo;

	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices);

};