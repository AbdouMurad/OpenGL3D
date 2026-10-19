#include "Buffer.h"
#include <iostream>

//VBO
VBO::VBO()
	: ID(0) {
}

void VBO::init(std::vector<Vertex>& vertices) {
	glGenBuffers(1, &ID);
	Bind();
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}
void VBO::Bind() {
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}
void VBO::Unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void VBO::Delete() {
	glDeleteBuffers(1, &ID);
}
void VBO::SetType(const void* data, GLsizeiptr size, GLenum usage) {
	Bind();
	glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

//EBO
EBO::EBO()
	: ID(0) {
}
void EBO::init(std::vector<GLuint>& indices) {
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data() , GL_STATIC_DRAW);
}
void EBO::Bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}
void EBO::Unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void EBO::Delete() {
	glDeleteBuffers(1, &ID);
}

//VAO
VAO::VAO()
	: ID(0) {}
void VAO::init() {
	glGenVertexArrays(1, &ID);
}
void VAO::LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponenets, GLenum type, GLsizeiptr stride, void* offset) {
	VBO.Bind();
	glVertexAttribPointer(layout, numComponenets, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);
	VBO.Unbind();
}
void VAO::Bind() {
	glBindVertexArray(ID);
}
void VAO::Unbind() {
	glBindVertexArray(0);
}
void VAO::Delete() {
	glDeleteVertexArrays(1, &ID);
}

//FBO
FBO::FBO()
	: ID(0) {}
void FBO::init() {
	glGenFramebuffers(1, &ID);
}
void FBO::Bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, ID);
}
void FBO::Unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void FBO::Delete() {
	glDeleteFramebuffers(1, &ID);
}
void FBO::AttachDepthTexture(GLuint textureID) {
	Bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);
}
void FBO::AttachDepthTextureLayer(GLuint textureID, GLint layer) {
	Bind();
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureID, 0, layer);
}
void FBO::DisableColorBuffers() {
	Bind();
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
}
bool FBO::IsComplete() {
	Bind();
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE) return true;

	std::cout << "FRAMEBUFFER INCOMPLETE: ";
	switch (status) {
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:		   std::cout << "an attachment is invalid"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: std::cout << "nothing attached"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:		   std::cout << "draw buffer has no attachment (depth only? call DisableColorBuffers)"; break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:		   std::cout << "read buffer has no attachment (depth only? call DisableColorBuffers)"; break;
	case GL_FRAMEBUFFER_UNSUPPORTED:				   std::cout << "format combination not supported"; break;
	default:										   std::cout << "status 0x" << std::hex << status << std::dec; break;
	}
	std::cout << std::endl;
	return false;
}