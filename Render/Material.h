#pragma once

#include "GPU/Texture.h"
#include "GPU/shaderClass.h"
#include "Core/Handlers.h"

struct MaterialTextures {
	TextureHandle baseColor = 0;
	TextureHandle normal = 0;
	TextureHandle metallicRoughness = 0;
	TextureHandle emissive = 0;
};
struct MaterialProperties {
	glm::vec4 baseColor = glm::vec4(1.0f);
	float metallic = 1.0f;
	float roughness = 1.0f;
	float normalScale = 1.0f;
	glm::vec3 emissiveFactor = glm::vec3(0.0f);
	float emissiveStrength = 1.0f; //KHR_materials_emissive_strength
};
struct Material {
	uint32_t flags = MATERIAL_NONE;

	MaterialTextures textures;
	MaterialProperties properties;

};

