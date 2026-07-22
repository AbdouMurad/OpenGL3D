#include "Renderer.h"

bool Renderer::init() {
	return true;
}

//void Renderer::BeginFrame() {}

void Renderer::Render(RenderFrame& frame) {
	currentFrame = &frame;
	for (RenderObject& object : currentFrame->renderObjects)
		Renderer::DrawNode(&(AssetManager::Get().GetModel(object.modelID).root), object.transform);
	currentFrame = nullptr;
}

void Renderer::DrawNode(Node* node, const glm::mat4& parentTransformMatrix) {
	glm::mat4 localTransformMatrix = node->localTransformComponent.getLocalMatrix();

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

		//Maybe move this into a function that takes in a material reference?
		shader.setVec4("u_baseColor", material.properties.baseColor);
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
			shader.setInt("u_hasNormalMap", NORMAL_SLOT);
			Texture& normalMapTex = AssetManager::Get().GetTexture(material.textures.normal);
			normalMapTex.Bind(NORMAL_SLOT);
		}
		else {
			shader.setInt("u_hasNormalMap", 0);
		}
		if (material.textures.metallicRoughness) {
			shader.setInt("u_hasMetallicRoughnessTex", METALLIC_ROUGHNESS_SLOT);
			Texture& mettalicRoughnessTex = AssetManager::Get().GetTexture(material.textures.metallicRoughness);
			mettalicRoughnessTex.Bind(METALLIC_ROUGHNESS_SLOT);
		}
		else {
			shader.setInt("u_hasMetallicRoughnessTex", 0);
		}


		//Lights
		glm::vec3 position = worldTransformMatrix[3];
		std::vector<RenderLight> lights;
		for (auto& light : currentFrame->lights) {
			if (glm::distance(position, light.position) < light.range) {
				lights.push_back(light);
			}
		}
		std::sort(lights.begin(), lights.end(), [position](const RenderLight& a, const RenderLight& b)
			{
				return glm::distance2(a.position, position) < glm::distance2(b.position, position);
			});
		shader.setInt("u_lightCount", lights.size());
		for (int i = 0; i < lights.size(); ++i) {
			std::string base = "u_lights[" + std::to_string(i) + "]";

			shader.setVec3(base + ".position", lights[i].position);
			shader.setVec3(base + ".color", lights[i].color);
			shader.setFloat(base + ".intensity", lights[i].intensity);
			shader.setFloat(base + ".range", lights[i].range);
		}
		glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
	}

	for (auto& c : node->children) {
		DrawNode(c.get(), worldTransformMatrix);
	}
}
