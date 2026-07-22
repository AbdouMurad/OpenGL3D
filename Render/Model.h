#pragma once

#include <memory>
#include <string>

#include "GPU/Buffer.h"
#include "Components/GameObject.h"
#include "Material.h"
#include "Core/Handlers.h"


class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	VAO VAO;
	EBO ebo;
	VBO vbo;

	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices);

};

struct Primitive {
	MeshHandle mesh = 0;
	MaterialHandle material = 0;
};

struct Node {
	TransformComponent localTransformComponent;

	Node* parent = nullptr;
	std::vector<std::unique_ptr<Node>> children;
	
	
	std::vector<Primitive> primitives;
	
	Node() = default;
	Node(Node* parentNode)
		: parent(parentNode) {}
};

class Model {
public:
	Node root;
};