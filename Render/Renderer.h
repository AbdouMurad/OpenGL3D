#pragma once

#include <memory>

#include "Model.h"
#include "Core/ShaderManager.h"


class Renderer {
public:
	bool init();
	void BeginFrame(const CameraComponent& camera);
	void Draw(Model& model, TransformComponent& transform);

private:
	glm::mat4 cameraMatrix = glm::mat4(1.0f);
	
	void DrawNode(Node* node, const glm::mat4& parentTransformComponent);
};