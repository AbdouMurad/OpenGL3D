#pragma once

#include <memory>

#include "Core/ShaderManager.h"

struct RenderLight {
	glm::vec3 position; //switch to mat4
	glm::vec3 color;
	float intensity;
	float range;
};

struct RenderDirectionalLight {
	glm::vec3 direction; //world space, normalized, the way the light travels
	glm::vec3 color;
	float intensity;
};

struct RenderObject {
	glm::mat4 transform;
	ModelHandle modelID;
};

struct RenderFrame {
	std::vector<RenderObject> renderObjects;
	std::vector<RenderLight> lights;
	std::vector<RenderDirectionalLight> directionalLights;
	glm::mat4 cameraMatrix = glm::mat4(1.0f);
	glm::vec3 cameraPosition = glm::vec3(0.0f);
};

struct RenderStats {
	int objectsDrawn = 0;
	int objectsCulled = 0; //outside the camera frustum
};

class Renderer {
public:

	bool init();
	//void BeginFrame();
	void Render(RenderFrame& frame);

	const RenderStats& GetStats() const { return stats; } //last frame

private:
	RenderFrame* currentFrame = nullptr;
	RenderStats stats;

	void DrawNode(Node* node, const glm::mat4& parentTransformComponent);
};