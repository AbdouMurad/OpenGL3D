#pragma once

#include <string>

#include "Buffer.h"
#include "Texture.h" 

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	VAO VAO;

	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture>& textures);

	//Draw() -> want this to be a renderer method that has a mesh passed into it rather than a method on mesh

};