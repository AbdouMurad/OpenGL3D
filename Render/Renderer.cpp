#include "Renderer.h"

bool Renderer::init() {
	return true;
}

void Renderer::Draw(Model& model, Transform& transform, Camera& camera) {
	glm::mat4 transformMat = glm::mat4(1.0f);
	
	transformMat = glm::translate(transformMat, transform.getPosition());
	transformMat *= glm::mat4_cast(transform.getQuat());
	transformMat = glm::scale(transformMat, transform.getSize());

	DrawNode(&(model.root), transformMat, camera);
}
//void Renderer::Draw(Mesh& mesh, Material& material) {
//	Shader& shader = *getShader(material.shaderType);
//	shader.Activate();
//	mesh.VAO.Bind();
//
//	glm::mat4 objectModel = glm::mat4(1.0f);
//	glm::vec3 objectRotation = glm::radians(mesh.transform.getRotation());
//	
//	objectModel = glm::translate(objectModel, mesh.transform.getPosition());
//	objectModel = glm::rotate(objectModel, objectRotation.x, glm::vec3(1,0,0));
//	objectModel = glm::rotate(objectModel, objectRotation.y, glm::vec3(0, 1, 0));
//	objectModel = glm::rotate(objectModel, objectRotation.z, glm::vec3(0, 0, 1));
//	objectModel = glm::scale(objectModel, mesh.transform.getSize());
//
//	shader.setMat4("camMatrix", currentCam->cameraMatrix);
//	shader.setMat4("model", objectModel);
//
//	material.Bind(shader);
//
//	glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
//}
void Renderer::DrawNode(Node* node, const glm::mat4& parentTransform, Camera& camera) {
	glm::mat4 localTransform = glm::mat4(1.0f);


	localTransform = glm::translate(localTransform, node->localTransform.getPosition());
	localTransform *= glm::mat4_cast(node->localTransform.getQuat());
	localTransform = glm::scale(localTransform, node->localTransform.getSize());

	glm::mat4 worldTransform = parentTransform * localTransform;
	for (auto& p : node->primitives) {
		Material& material = AssetManager::Get().GetMaterial(p.material);
		
		Mesh& mesh = AssetManager::Get().GetMesh(p.mesh);
		Shader& shader = AssetManager::Get().GetShader(ShaderManager::Get().GetMaterialShader(material.flags));

		shader.Activate();
		mesh.VAO.Bind();

		shader.setMat4("camMatrix", camera.cameraMatrix);
		shader.setMat4("model", worldTransform);

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
		DrawNode(c.get(), worldTransform, camera);
	}
}
