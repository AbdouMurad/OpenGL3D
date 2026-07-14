#pragma once

#include <memory>

#include "Model.h" //TODO: Move model to its own component folder?
#include "Components/Camera.h"
#include "Material.h"
#include "GPU/shaderClass.h"

#include "Core/ShaderManager.h"

class Renderer {
public:

	bool init();


	//void Draw(Mesh& mesh, Material& material);
	void Draw(Model& model, Transform& transform, Camera& camera);


private:

	void DrawNode(Node* node, const glm::mat4& parentTransform, Camera& camera);
};