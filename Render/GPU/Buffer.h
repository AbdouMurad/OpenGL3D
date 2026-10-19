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

	void SetType(const void* data, GLsizeiptr size, GLenum usage);
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
	void init();

	void LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponenets, GLenum type, GLsizeiptr stride, void* offset);
	void Bind();
	void Unbind();
	void Delete();
};

//framebuffer: something to render into other than the window (e.g. a shadow map's depth texture)
class FBO {
public:
	GLuint ID;
	FBO();
	void init();

	void Bind();	//following draws go into this framebuffer
	void Unbind();	//back to the window
	void Delete();

	//texture becomes the depth target. AttachDepthTextureLayer is for one layer of a texture array (one shadow map per light)
	void AttachDepthTexture(GLuint textureID);
	void AttachDepthTextureLayer(GLuint textureID, GLint layer);
	//depth only targets draw/read no colour, OpenGL has to be told or the framebuffer is incomplete
	void DisableColorBuffers();

	//attachments fit together and it can be rendered to. prints the reason when not
	bool IsComplete();
};
