#pragma once

#include <memory>

#include "Model.h" //TODO: Move model to its own component folder?
#include "Core/Camera.h"
#include "shaderClass.h"

//Keep COUNT as last so ShaderType::COUNT returns count of enums
enum ShaderType {
	DEFAULT,
	COUNT
};

class Renderer {
public:
	std::unique_ptr<Shader> shaders[ShaderType::COUNT];

	~Renderer();
	Renderer();
	bool init();
	Shader* getShader(ShaderType shaderType);
	void Draw(Mesh& mesh, Shader& shader, Camera& camera);

};