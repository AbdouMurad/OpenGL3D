#include "ShadowMap.h"
#include <iostream>

ShadowMap::ShadowMap()
	: textureID(0), size(0), layers(0), savedViewport{ 0, 0, 0, 0 } {}

bool ShadowMap::init(int s, int l) {
	GLint maxSize = 0;
	GLint maxLayers = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxLayers);
	if (s <= 0 || s > maxSize || l <= 0 || l > maxLayers) {
		std::cout << "SHADOW MAP: size " << s << " x " << l << " layers not supported (max " << maxSize << " x " << maxLayers << " layers)" << std::endl;
		return false;
	}
	size = s;
	layers = l;

	//one texture holding every layer. depth only, 24 bit precision, no data yet (rendered into every frame)
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, size, size, layers, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	//nearest: the shader compares depths itself, blending depths of different surfaces would be meaningless
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//outside the light's view nothing is known -> treat as depth 1.0 (furthest), so it counts as lit
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	const float border[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	//framebuffer that renders into one layer at a time, depth only
	fbo.init();
	fbo.DisableColorBuffers();
	fbo.AttachDepthTextureLayer(textureID, 0);
	bool complete = fbo.IsComplete();
	fbo.Unbind();
	if (!complete) {
		std::cout << "SHADOW MAP: framebuffer could not be completed" << std::endl;
		Delete();
		return false;
	}
	return true;
}

void ShadowMap::BeginLayer(int layer) {
	if (layer < 0 || layer >= layers) {
		std::cout << "SHADOW MAP: layer " << layer << " out of range (0.." << layers - 1 << ")" << std::endl;
		return;
	}
	//remember the window's viewport so End() can put it back
	glGetIntegerv(GL_VIEWPORT, savedViewport);

	fbo.AttachDepthTextureLayer(textureID, layer); //also binds the framebuffer
	glViewport(0, 0, size, size);
	glClearDepth(1.0);	//1.0 = "nothing in the way" until something is drawn
	glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::End() {
	fbo.Unbind();
	glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);
}

void ShadowMap::Bind(GLuint slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
}

void ShadowMap::Delete() {
	fbo.Delete();
	glDeleteTextures(1, &textureID);
	textureID = 0;
	size = 0;
	layers = 0;
}
