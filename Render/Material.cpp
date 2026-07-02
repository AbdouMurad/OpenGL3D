#include "Material.h"

Material::Material(ShaderType shader)
	:	shaderType(shader) {}

UnlitMaterial::UnlitMaterial(const char* texturePath)
	:	texture(texturePath, "UNLIT", 0, GL_RGBA, GL_UNSIGNED_BYTE), Material(ShaderType::UNLIT) { }

void UnlitMaterial::Bind(Shader& shader) {
	texture.texUnit(shader, "tex", 0);
	texture.Bind();
}

