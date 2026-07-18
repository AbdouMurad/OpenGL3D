#include "Renderer.h"

bool Renderer::init() {
	return true;
}

//void Renderer::BeginFrame() {}

void Renderer::Render(RenderFrame& frame) {
	currentFrame = &frame;
	for (RenderObject& object : frame.renderObjects)
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
		shader.setMat4("model", worldTransformMatrix);

		//Maybe move this into a function that takes in a material reference?
		shader.setVec4("u_baseColor", material.properties.baseColor);
		if (material.textures.baseColor) {
			shader.setInt("u_hasBaseColorTex", 1);
			Texture& baseColorTex = AssetManager::Get().GetTexture(material.textures.baseColor);
			baseColorTex.Bind();
		}
		else {
			shader.setInt("u_hasBaseColorTex", 0);
		}

		//Lights
		shader.setInt("u_lightCount", currentFrame->lights.size());
		for (int i = 0; i < currentFrame->lights.size(); ++i) {
			std::string base = "u_lights[" + std::to_string(i) + "]";

			shader.setVec3(base + ".position", currentFrame->lights[i].position);
			shader.setVec3(base + ".color", currentFrame->lights[i].color);
			shader.setFloat(base + ".intensity", currentFrame->lights[i].intensity);
		}
		glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
	}

	for (auto& c : node->children) {
		DrawNode(c.get(), worldTransformMatrix);
	}
}
