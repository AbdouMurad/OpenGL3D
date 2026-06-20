#pragma once

#include <glm/glm.hpp>
#include "gl_core.h"

#include <vector>

class VBO {
public:
	GLuint ID;
	VBO(); //only used for renderer
	void init(GLfloat* vertices, GLsizeiptr size);

	void Bind();
	void Unbind();
	void Delete();
};

class EBO {
public:
	GLuint ID;
	EBO(); //only used to init renderer
	void init(GLuint* indices, GLsizeiptr size);

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
