#pragma once

#include <memory>

#include "Core/ShaderManager.h"
#include "GPU/ShadowMap.h"

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
	int objectsInShadowPass = 0; //drawn into the shadow map (inside the light's box)
};

class Renderer {
public:

	bool init();
	//void BeginFrame();
	void Render(RenderFrame& frame);

	const RenderStats& GetStats() const { return stats; } //last frame

	//shows the shadow map in the bottom right corner (dark = close to the light)
	void ToggleShadowDebug() { showShadowMap = !showShadowMap; }

	//area the sun's shadows cover: a box around shadowCenter reaching shadowRadius in every direction
	//anything outside gets no shadows. bigger = more area but blurrier shadows (same number of texels)
	glm::vec3 shadowCenter = glm::vec3(0.0f);
	float shadowRadius = 20.0f;

private:
	RenderFrame* currentFrame = nullptr;
	RenderStats stats;

	void DrawNode(Node* node, const glm::mat4& parentTransformComponent);

	//---shadows---
	ShadowMap shadowMap;
	bool shadowsReady = false;			   //shadow map created (false = render without shadows)
	bool shadowsThisFrame = false;		   //shadow pass ran this frame -> pass 2 reads the map
	Shader* shadowShader = nullptr;		   //depth only
	Shader* shadowDebugShader = nullptr;   //shadow map as grey
	glm::mat4 lightMatrix = glm::mat4(1.0f); //projection * view of the shadow casting light, also needed by pass 2
	bool showShadowMap = false;
	VAO debugQuadVAO;
	VBO debugQuadVBO;

	glm::mat4 ComputeLightMatrix(const glm::vec3& lightDirection) const;
	void RenderShadowPass(const RenderDirectionalLight& light);
	void DrawNodeDepth(Node* node, const glm::mat4& parentMatrix);
	void DrawShadowMapDebug();
};