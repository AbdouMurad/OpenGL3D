#include "Material.h"

Material::Material(ShaderHandle shader)
	:	shader(shader) {}

UnlitMaterial::UnlitMaterial(const char* texturePath, ShaderHandle shader)
	:	Material(shader) {}

void UnlitMaterial::Bind(Shader& shader) {
	Texture textureObj = AssetManager::Get().GetTexture(texture);
	textureObj.texUnit(shader, "tex", 0);
	textureObj.Bind();
}
