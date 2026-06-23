#pragma once

#include "Model.h" //TODO: Move model to its own component folder?
#include "Core/Camera.h"
#include "shaderClass.h"

enum ShaderType {
	DEFAULT
};

class Renderer {
public:
	Shader defaultShader;

	~Renderer();
	Renderer();
	//bool init();
	Shader* getShader(ShaderType shaderType);
	void Draw(Mesh& mesh, Shader& shader, Camera& camera);
};