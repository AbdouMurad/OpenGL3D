#pragma once

#include "GPU/Texture.h"
#include "GPU/shaderClass.h"
#include "Core/AssetManager.h"

class Material {
public:
	ShaderHandle shader = 0; 

	Material(ShaderHandle shader);
	virtual ~Material() = default;
	
	virtual void Bind(Shader& shader) = 0;
};

class UnlitMaterial : public Material {
public:
	UnlitMaterial(const char* texturePath, ShaderHandle shader);
	~UnlitMaterial() = default;

	TextureHandle texture = 0;

	void Bind(Shader& shader) override;

};
