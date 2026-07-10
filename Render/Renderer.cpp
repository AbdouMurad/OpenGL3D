#include "Renderer.h"



bool Renderer::init() {
	return true;
}

void Renderer::Draw(Mesh& mesh, Material& material) {
	Shader& shader = *getShader(material.shaderType);
	shader.Activate();
	mesh.VAO.Bind();

	glm::mat4 objectModel = glm::mat4(1.0f);
	glm::vec3 objectRotation = glm::radians(mesh.transform.getRotation());
	
	objectModel = glm::translate(objectModel, mesh.transform.getPosition());
	objectModel = glm::rotate(objectModel, objectRotation.x, glm::vec3(1,0,0));
	objectModel = glm::rotate(objectModel, objectRotation.y, glm::vec3(0, 1, 0));
	objectModel = glm::rotate(objectModel, objectRotation.z, glm::vec3(0, 0, 1));
	objectModel = glm::scale(objectModel, mesh.transform.getSize());

	shader.setMat4("camMatrix", currentCam->cameraMatrix);
	shader.setMat4("model", objectModel);

	material.Bind(shader);

	glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
}
void Renderer::Draw(GameObject& ob) {
	
}
void Renderer::DrawNode(Node& node, const glm::mat4 parentTransform) {

}
