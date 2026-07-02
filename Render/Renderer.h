#pragma once

#include <memory>

#include "Model.h" //TODO: Move model to its own component folder?
#include "Components/Camera.h"
#include "Material.h"
#include "GPU/shaderClass.h"
#include "ShaderTypes.h"

class Renderer {
public:
	std::unique_ptr<Shader> shaders[ShaderType::COUNT];

	bool init();
	Shader* getShader(ShaderType shaderType);
	void Draw(Mesh& mesh, Material& material, Camera& camera);

};