#include "Material.h"

Material::Material(ShaderHandle shader)
	:	shader(shader) {}

UnlitMaterial::UnlitMaterial(const char* texturePath, ShaderHandle shader)
	:	Material(shader) { 

}

void UnlitMaterial::Bind(Shader& shader) {
	texture.texUnit(shader, "tex", 0);
	texture.Bind();
}

