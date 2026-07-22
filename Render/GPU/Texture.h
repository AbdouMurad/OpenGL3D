#pragma once

#include "Core/gl_core.h"
#include <stb_image.h>

#include"shaderClass.h"
enum TextureSlot {
	BASE_COLOR_SLOT = 0,
	NORMAL_SLOT = 1,
	METALLIC_ROUGHNESS_SLOT = 2,
	EMISSIVE_SLOT = 3
};
enum class TextureType
{
	BaseColor,
	Normal,
	MetallicRoughness,
	Emissive
};
class Texture
{
public:
	GLuint ID = 0;
	TextureType type;

	~Texture();
	Texture(const char* image, TextureType type, GLenum format, GLenum pixelType);
	Texture(const char* image, TextureType type);

	// Assigns a texture unit to a texture
	void texUnit(Shader& shader, const char* uniform, GLuint unit);
	// Binds a texture
	void Bind(GLuint slot);
	// Unbinds a texture
	void Unbind();
	// Deletes a texture
	void Delete();
};