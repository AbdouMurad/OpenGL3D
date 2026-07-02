#pragma once

#include "ShaderTypes.h"
#include "GPU/Texture.h"
#include "GPU/shaderClass.h"

class Material {
public:
	ShaderType shaderType;

	Material(ShaderType type);
	virtual ~Material() = default;
	
	virtual void Bind(Shader& shader) = 0;
};

class UnlitMaterial : public Material {
public:
	UnlitMaterial(const char* texturePath);
	~UnlitMaterial() = default;

	Texture texture;

	void Bind(Shader& shader) override;

};