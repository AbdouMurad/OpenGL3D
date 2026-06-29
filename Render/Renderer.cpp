#include "Renderer.h"

Renderer::Renderer() {}

Renderer::~Renderer() {}

Shader* Renderer::getShader(ShaderType shaderType) {
	return shaders[shaderType].get();
}

bool Renderer::init() {
	shaders[ShaderType::DEFAULT] = std::make_unique<Shader>("assets/shaders/default.vert", "assets/shaders/default.frag");
	return true;
}

void Renderer::Draw(Mesh& mesh, Shader& shader, Camera& camera) {
	shader.Activate();
	mesh.VAO.Bind();

	unsigned int numDiffuser = 0;
	unsigned int numSpecular = 0;

	for (unsigned int i = 0; i < mesh.textures.size(); i++)
	{
		std::string num;
		std::string type = mesh.textures[i].type;
		if (type == "diffuse") {
			num = std::to_string(numDiffuser++);
		}
		else if (type == "specular") {
			num = std::to_string(numSpecular++);
		}

		mesh.textures[i].texUnit(shader, (type + num).c_str(), i);
		mesh.textures[i].Bind();
	}
	shader.setVec3("camPos", camera.Position);
	shader.setMat4("camMatrix", camera.cameraMatrix);

	glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
}