#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp> 
#include <cmath>
#include <iostream>
#include "Core/Handlers.h"

class Renderer;
class GameObject;

class Component {
protected:
	GameObject* owner = nullptr;

public:
	virtual ~Component() = default;

	void SetOwner(GameObject* obj) {
		owner = obj;
	}

	virtual void Start() {}
	virtual void Update(float) {}

};

class TransformComponent : public Component {
private:
	glm::vec3 position = glm::vec3(0,0,0);
	glm::quat rotation = glm::quat(1,0,0,0);
	glm::vec3 size = glm::vec3(1, 1, 1);
public:
	TransformComponent(glm::vec3 position, glm::vec3 size, glm::vec3 rotation);
	TransformComponent(glm::vec3 position);
	TransformComponent() = default;

	glm::vec3 getPosition();
	glm::vec3 getSize();
	glm::vec3 getRotation();
	glm::quat getQuat();

	glm::mat4 getMatrix() const;

	void setPosition(glm::vec3 pos);
	void setSize(glm::vec3 size);
	void setRotation(glm::vec3 rotation);
	void setRotation(glm::quat rotation);

	void translate(glm::vec3 delta);
	void rotate(glm::vec3 rotation);
	void scale(float scale);

	friend std::ostream& operator<<(std::ostream& os, const TransformComponent& transform);
};

class MeshRenderer : public Component {
public:
	ModelHandle modelID = 0;
	MeshRenderer(ModelHandle m);

	void Render(Renderer& renderer);
};

class GameObject {
	std::vector<std::unique_ptr<Component>> components;

public: 
	template<typename T, typename... Args>
	T* AddComponent(Args&&... args) {
		auto component = std::make_unique<T>(std::forward<Args>(args)...);

		T* ptr = component.get();

		ptr->SetOwner(this);

		components.push_back(std::move(component));

		ptr->Start();

		return ptr;
	}

	template<typename T>
	T* GetComponent() {
		for (auto& component : components) {
			if (auto cast = dynamic_cast<T*>(component.get()))
				return cast;
		}

		return nullptr;
	}

	void Update(float dt) {
		for (auto& component : components) {
			component->Update(dt);
		}
	}
};


