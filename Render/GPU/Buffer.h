#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Core/gl_core.h"

struct Vertex {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 normal = glm::vec3(0.0f);
	glm::vec2 texUV = glm::vec2(0.0f);
	glm::vec4 tangent = glm::vec4(0.0f);
};

struct TangentData {
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

class VBO {
public:
	GLuint ID;
	VBO(); //only used for renderer
	void init(std::vector<Vertex>& vertices);

	void Bind();
	void Unbind();
	void Delete();
};

class EBO {
public:
	GLuint ID;
	EBO(); //only used to init renderer
	void init(std::vector<GLuint>& indices);

	void Bind();
	void Unbind();
	void Delete();
};

class VAO {
public:
	GLuint ID;
	VAO();

	void LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponenets, GLenum type, GLsizeiptr stride, void* offset);
	void Bind();
	void Unbind();
	void Delete();
};
