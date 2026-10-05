#pragma once

#include <memory>

#include "Core/ShaderManager.h"

struct RenderLight {
	glm::vec3 position; //switch to mat4
	glm::vec3 color;
	float intensity;
	float range;
};

struct RenderObject {
	glm::mat4 transform;
	ModelHandle modelID;
	glm::vec3 tint = glm::vec3(1.0f);
	glm::vec3 emissive = glm::vec3(0.0f);
	bool overlay = false;
};

struct RenderFrame {
	std::vector<RenderObject> renderObjects;
	std::vector<RenderLight> lights;
	glm::mat4 cameraMatrix = glm::mat4(1.0f);
	glm::vec3 cameraPosition = glm::vec3(0.0f);
};

class Renderer {
public:

	bool init();
	//void BeginFrame();
	void Render(RenderFrame& frame);

private:
	RenderFrame* currentFrame = nullptr;
	RenderObject* currentObject = nullptr;

	void DrawNode(Node* node, const glm::mat4& parentTransformComponent);
};