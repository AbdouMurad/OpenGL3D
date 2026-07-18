#include "Renderer.h"

bool Renderer::init() {
	return true;
}

void Renderer::BeginFrame() {}

void Renderer::Render(RenderFrame& frame) {
	for (RenderObject& object : frame.renderObjects)
		Renderer::DrawNode(&(AssetManager::Get().GetModel(object.modelID).root), object.transform, frame.cameraMatrix);
}

void Renderer::DrawNode(Node* node, const glm::mat4& parentTransformComponent, const glm::mat4& cameraMatrix) {
	glm::mat4 localTransformComponent = node->localTransformComponent.getLocalMatrix();

	glm::mat4 worldTransformComponent = parentTransformComponent * localTransformComponent;
	for (auto& p : node->primitives) {
		Material& material = AssetManager::Get().GetMaterial(p.material);
		Mesh& mesh = AssetManager::Get().GetMesh(p.mesh);

		Shader& shader = AssetManager::Get().GetShader(ShaderManager::Get().GetMaterialShader(material.flags));

		shader.Activate();
		mesh.VAO.Bind();

		shader.setMat4("camMatrix", cameraMatrix);
		shader.setMat4("model", worldTransformComponent);

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

		glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
	}

	for (auto& c : node->children) {
		DrawNode(c.get(), worldTransformComponent, cameraMatrix);
	}
}
