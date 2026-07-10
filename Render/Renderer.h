#pragma once

#include <memory>

#include "Model.h" //TODO: Move model to its own component folder?
#include "Components/Camera.h"
#include "Material.h"
#include "GPU/shaderClass.h"


class Renderer {
public:

	bool init();

	void setCamera(Camera* cam) { currentCam = cam; }

	void Draw(Mesh& mesh, Material& material);
	void Draw(GameObject& object);

private:
	Camera* currentCam;
	void DrawNode(Node& node, const glm::mat4 parentTransform);
};