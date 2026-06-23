#include "Renderer.h"

Renderer::Renderer() 
	:	defaultShader("default.vert", "default.frag") {}

Renderer::~Renderer() {
	defaultShader.Delete();
}

Shader* Renderer::getShader(ShaderType shaderType) {
	switch (shaderType) {
	case ShaderType::DEFAULT:
		return &defaultShader;
	} 
	return nullptr;
}

void Renderer::Draw(Mesh& mesh, Shader& shader, Camera& camera) {
	
}