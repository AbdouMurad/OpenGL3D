#pragma once

#include "Core/gl_core.h"
#include "Buffer.h"

//depth images rendered from the lights' point of view, one layer per shadow casting light
//pass 1 (per light): BeginLayer(i) -> draw the scene with a depth only shader -> End()
//pass 2: Bind(slot) -> shaders sample it as a sampler2DArray, layer i belongs to light i
class ShadowMap {
public:
	GLuint textureID;	//GL_TEXTURE_2D_ARRAY with `layers` depth images of size x size
	int size;			//width = height of every layer, in texels (more = sharper shadows, more memory)
	int layers;			//how many lights can cast shadows at once

	ShadowMap();
	//creates the texture array + framebuffer. false (and a printed reason) if the GPU can't do it
	bool init(int size, int layers);

	//pass 1: following draws write depth into this layer. sets the viewport to the layer size and clears it
	void BeginLayer(int layer);
	//pass 1 done: back to the window, with the viewport it had before BeginLayer
	void End();

	//pass 2: binds the whole array to a texture unit for reading in shaders
	void Bind(GLuint slot);
	void Delete();

private:
	FBO fbo;
	GLint savedViewport[4];	//window viewport, restored by End()
};
