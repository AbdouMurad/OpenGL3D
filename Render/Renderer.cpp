#include "Renderer.h"


Shader* Renderer::getShader(ShaderType shaderType) {
	return shaders[shaderType].get();
}

bool Renderer::init() {
	shaders[ShaderType::UNLIT] = std::make_unique<Shader>("assets/shaders/unlit.vert", "assets/shaders/unlit.frag");
	return true;
}

void Renderer::Draw(Mesh& mesh, Material& material, Camera& camera) {
	Shader& shader = *getShader(material.shaderType);
	shader.Activate();
	mesh.VAO.Bind();

	glm::mat4 objectModel = glm::mat4(1.0f);
	objectModel = glm::translate(objectModel, mesh.position);

	shader.setMat4("camMatrix", camera.cameraMatrix);
	shader.setMat4("model", objectModel);

	material.Bind(shader);

	glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
}