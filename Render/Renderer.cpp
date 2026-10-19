#include "Renderer.h"
#include <cstddef>

bool Renderer::init() {
	//shadows: one layer for now (the first directional light). the array leaves room for more casters later
	constexpr int shadowMapSize = 2048;
	constexpr int shadowCasters = 1;
	shadowsReady = shadowMap.init(shadowMapSize, shadowCasters);

	shadowShader = &AssetManager::Get().GetShader(AssetManager::Get().LoadShader("assets/shaders/shadow.vert", "assets/shaders/shadow.frag"));
	shadowDebugShader = &AssetManager::Get().GetShader(AssetManager::Get().LoadShader("assets/shaders/shadow_debug.vert", "assets/shaders/shadow_debug.frag"));

	//debug quad covering all of clip space, uv 0..1. DrawShadowMapDebug shrinks it into a corner with the viewport
	//counter clockwise, so face culling keeps it
	const glm::vec3 positions[6] = { {-1, -1, 0}, {1, -1, 0}, {1, 1, 0}, {-1, -1, 0}, {1, 1, 0}, {-1, 1, 0} };
	const glm::vec2 uvs[6] = { {0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1} };
	std::vector<Vertex> quad(6);
	for (int i = 0; i < 6; ++i) {
		quad[i].position = positions[i];
		quad[i].texUV = uvs[i];
	}
	debugQuadVAO.init();
	debugQuadVAO.Bind();
	debugQuadVBO.init(quad);
	debugQuadVAO.LinkAttrib(debugQuadVBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	debugQuadVAO.LinkAttrib(debugQuadVBO, 1, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, texUV));
	debugQuadVAO.Unbind();

	return true;
}

//void Renderer::BeginFrame() {}

void Renderer::Render(RenderFrame& frame) {
	currentFrame = &frame;
	stats = RenderStats();

	//pass 1: depth from the first directional light, before anything is drawn to the window
	shadowsThisFrame = shadowsReady && !frame.directionalLights.empty();
	if (shadowsThisFrame) {
		RenderShadowPass(frame.directionalLights[0]);
	}

	//pass 2: the normal render from the camera
	//shadow map bound once for the whole pass (materials use units 0-3, so nothing else touches unit 4)
	if (shadowsReady) shadowMap.Bind(SHADOW_MAP_SLOT);
	Frustum frustum;
	frustum.Extract(frame.cameraMatrix);

	for (RenderObject& object : currentFrame->renderObjects) {
		Model& model = AssetManager::Get().GetModel(object.modelID);
		//skip before any uniforms/draw calls happen. models without bounds (no meshes) are never culled
		if (model.bounds.IsValid() && !frustum.IsVisible(model.bounds.Transformed(object.transform))) {
			++stats.objectsCulled;
			continue;
		}
		++stats.objectsDrawn;
		Renderer::DrawNode(&model.root, object.transform);
	}

	if (showShadowMap && shadowsReady) {
		DrawShadowMapDebug();
	}
	currentFrame = nullptr;
}

//-------------------Shadows-----------------
//the light as a camera: placed behind the shadow box, looking along the light direction, orthographic
//(sun rays are parallel, so a box instead of a perspective pyramid)
glm::mat4 Renderer::ComputeLightMatrix(const glm::vec3& lightDirection) const {
	glm::vec3 direction = glm::normalize(lightDirection);
	//lookAt needs an up vector that isn't parallel to the view direction (a sun pointing straight down)
	glm::vec3 up = std::abs(direction.y) > 0.99f ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);

	glm::vec3 eye = shadowCenter - direction * (shadowRadius * 2.0f);
	glm::mat4 view = glm::lookAt(eye, shadowCenter, up);
	//near 0.1 keeps tall casters between the light and the box. the box ends at 2r + r = 3r from the eye
	glm::mat4 projection = glm::ortho(-shadowRadius, shadowRadius, -shadowRadius, shadowRadius, 0.1f, shadowRadius * 3.0f);
	return projection * view;
}

void Renderer::RenderShadowPass(const RenderDirectionalLight& light) {
	lightMatrix = ComputeLightMatrix(light.direction);
	//same culling as the camera pass, but against the light's box: objects outside it can't cast into it
	Frustum lightFrustum;
	lightFrustum.Extract(lightMatrix);

	shadowMap.BeginLayer(0);
	//draw back faces into the map: the lit surface then isn't compared against its own depth -> far less acne
	//(works for closed meshes, a single sided plane would cast no shadow)
	glCullFace(GL_FRONT);
	shadowShader->Activate();
	shadowShader->setMat4("u_lightMatrix", lightMatrix);

	for (RenderObject& object : currentFrame->renderObjects) {
		Model& model = AssetManager::Get().GetModel(object.modelID);
		if (model.bounds.IsValid() && !lightFrustum.IsVisible(model.bounds.Transformed(object.transform))) continue;
		++stats.objectsInShadowPass;
		DrawNodeDepth(&model.root, object.transform);
	}

	glCullFace(GL_BACK);
	shadowMap.End();
}

//like DrawNode, but only positions: no materials, textures or lights
void Renderer::DrawNodeDepth(Node* node, const glm::mat4& parentMatrix) {
	glm::mat4 worldMatrix = parentMatrix * node->localTransformComponent.GetLocalMatrix();
	for (auto& p : node->primitives) {
		Mesh& mesh = AssetManager::Get().GetMesh(p.mesh);
		shadowShader->setMat4("model", worldMatrix);
		mesh.VAO.Bind();
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
	}
	for (auto& child : node->children) {
		DrawNodeDepth(child.get(), worldMatrix);
	}
}

//layer 0 as a grey square in the bottom right corner, on top of everything
void Renderer::DrawShadowMapDebug() {
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	GLint side = viewport[3] / 3;
	GLint margin = 10;
	glViewport(viewport[0] + viewport[2] - side - margin, viewport[1] + margin, side, side);
	glDisable(GL_DEPTH_TEST);

	shadowDebugShader->Activate();
	shadowMap.Bind(SHADOW_MAP_SLOT);
	shadowDebugShader->setInt("u_shadowMap", SHADOW_MAP_SLOT);
	shadowDebugShader->setInt("u_layer", 0);
	debugQuadVAO.Bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	debugQuadVAO.Unbind();

	glEnable(GL_DEPTH_TEST);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void Renderer::DrawNode(Node* node, const glm::mat4& parentTransformMatrix) {
	glm::mat4 localTransformMatrix = node->localTransformComponent.GetLocalMatrix();

	glm::mat4 worldTransformMatrix = parentTransformMatrix * localTransformMatrix;
	for (auto& p : node->primitives) {
		Material& material = AssetManager::Get().GetMaterial(p.material);
		Mesh& mesh = AssetManager::Get().GetMesh(p.mesh);

		Shader& shader = AssetManager::Get().GetShader(ShaderManager::Get().GetMaterialShader(material.flags));

		shader.Activate();
		mesh.VAO.Bind();

		shader.setMat4("camMatrix", currentFrame->cameraMatrix);
		shader.setVec3("u_cameraPos", currentFrame->cameraPosition);
		shader.setMat4("model", worldTransformMatrix);

		//shadows. u_shadowMap is set even when off: left at its default (unit 0) it would share a unit with
		//u_baseColorTex, and two sampler types on one unit is an error in a draw call
		shader.setInt("u_shadowMap", SHADOW_MAP_SLOT);
		shader.setInt("u_hasShadowMap", shadowsThisFrame ? 1 : 0);
		shader.setMat4("u_lightMatrix", lightMatrix);

		//Maybe move this into a function that takes in a material reference?
		shader.setVec4("u_baseColor", material.properties.baseColor);
		shader.setFloat("u_metallic", material.properties.metallic);
		shader.setFloat("u_roughness", material.properties.roughness);
		shader.setFloat("u_normalScale", material.properties.normalScale);
		shader.setVec3("u_emissiveFactor", material.properties.emissiveFactor * material.properties.emissiveStrength);
		if (material.textures.baseColor) {
			shader.setInt("u_hasBaseColorTex", 1);
			shader.setInt("u_baseColorTex", BASE_COLOR_SLOT);
			Texture& baseColorTex = AssetManager::Get().GetTexture(material.textures.baseColor);
			baseColorTex.Bind(BASE_COLOR_SLOT);
		}
		else {
			shader.setInt("u_hasBaseColorTex", 0);
		}
		if (material.textures.normal) {
			shader.setInt("u_hasNormalTex", 1);
			shader.setInt("u_normalTex", NORMAL_SLOT);
			Texture& normalMapTex = AssetManager::Get().GetTexture(material.textures.normal);
			normalMapTex.Bind(NORMAL_SLOT);
		}
		else {
			shader.setInt("u_hasNormalTex", 0);
		}
		if (material.textures.metallicRoughness) {
			shader.setInt("u_hasMetallicRoughnessTex", 1);
			shader.setInt("u_metallicRoughnessTex", METALLIC_ROUGHNESS_SLOT);
			Texture& mettalicRoughnessTex = AssetManager::Get().GetTexture(material.textures.metallicRoughness);
			mettalicRoughnessTex.Bind(METALLIC_ROUGHNESS_SLOT);
		}
		else {
			shader.setInt("u_hasMetallicRoughnessTex", 0);
		}
		if (material.textures.emissive) {
			shader.setInt("u_hasEmissiveTex", 1);
			shader.setInt("u_emissiveTex", EMISSIVE_SLOT);
			Texture& emissiveTex = AssetManager::Get().GetTexture(material.textures.emissive);
			emissiveTex.Bind(EMISSIVE_SLOT);
		}
		else {
			shader.setInt("u_hasEmissiveTex", 0);
		}


		//Lights
		glm::vec3 position = worldTransformMatrix[3];
		std::vector<RenderLight> lights;
		for (auto& light : currentFrame->lights) {
			if (glm::distance(position, light.position) < light.range * 3.0f) {
				lights.push_back(light);
			}
		}
		std::sort(lights.begin(), lights.end(), [position](const RenderLight& a, const RenderLight& b)
			{
				return glm::distance2(a.position, position) < glm::distance2(b.position, position);
			});
		int lightCount = static_cast<int>(std::min<size_t>(lights.size(), 16));
		shader.setInt("u_lightCount", lightCount);
		for (int i = 0; i < lightCount; ++i) {
			std::string base = "u_lights[" + std::to_string(i) + "]";

			shader.setVec3(base + ".position", lights[i].position);
			shader.setVec3(base + ".color", lights[i].color);
			shader.setFloat(base + ".intensity", lights[i].intensity);
			shader.setFloat(base + ".range", lights[i].range);
		}

		//directional lights reach everything -> no distance filtering or sorting
		int directionalCount = static_cast<int>(std::min<size_t>(currentFrame->directionalLights.size(), 4));
		shader.setInt("u_directionalLightCount", directionalCount);
		for (int i = 0; i < directionalCount; ++i) {
			const RenderDirectionalLight& light = currentFrame->directionalLights[i];
			std::string base = "u_directionalLights[" + std::to_string(i) + "]";

			shader.setVec3(base + ".direction", light.direction);
			shader.setVec3(base + ".color", light.color);
			shader.setFloat(base + ".intensity", light.intensity);
		}
		glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
	}

	for (auto& c : node->children) {
		DrawNode(c.get(), worldTransformMatrix);
	}
}
