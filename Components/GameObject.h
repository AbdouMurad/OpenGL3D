#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp> 
#include <cmath>
#include <iostream>
#include <type_traits>
#include "Core/Handlers.h"

class GameObject;
class RigidBodyComponent;

class Component {
protected:
	std::string ID = "";
	GameObject* owner = nullptr;

public:
	virtual ~Component() = default;

	void SetOwner(GameObject* obj) {
		owner = obj;
	}
	GameObject* getOwner() const {
		return owner;
	};
	void SetID(std::string s) {
		ID = s;
	}
	const std::string& GetID() const {
		return ID;
	}
	virtual void Start() {}
	virtual void Update(float dt) {}

};

struct Transform {
	glm::vec3 position = glm::vec3(0);
	glm::quat rotation = glm::quat(1, 0, 0, 0);
	glm::vec3 size = glm::vec3(1);

	glm::mat4 GetMatrix() const;
};

class TransformComponent : public Component {
private:
	Transform local;
public:
	TransformComponent* parent = nullptr;

	TransformComponent(glm::vec3 position, glm::vec3 size, glm::vec3 rotation);
	TransformComponent(glm::vec3 position);
	TransformComponent() = default;

	glm::vec3 GetLocalPosition() const;
	glm::vec3 GetPosition() const;

	glm::vec3 GetLocalSize() const;
	glm::vec3 GetSize() const;

	glm::vec3 GetLocalRotation() const;
	glm::vec3 GetRotation() const;

	glm::quat GetLocalQuat() const;
	glm::quat GetQuat() const;

	glm::mat4 GetLocalMatrix() const;
	glm::mat4 GetMatrix() const;

	glm::vec3 Forward() const;
	glm::vec3 Right() const;
	glm::vec3 Up() const;

	void SetPosition(const glm::vec3& pos);
	void SetSize(const glm::vec3& size);
	void SetRotation(const glm::vec3& rotation);
	void SetRotation(const glm::quat& rotation);

	void Translate(const glm::vec3& delta);
	void Rotate(const glm::vec3& rotation);
	void Rotate(const glm::quat& rotation);
	void Scale(float scale);

	friend std::ostream& operator<<(std::ostream& os, const TransformComponent& transform);
};

class MeshRenderer : public Component {
public:
	ModelHandle modelID = 0;
	MeshRenderer(ModelHandle m);
};

class GameObject {
	ObjectHandle ID;
	std::vector<std::unique_ptr<Component>> components;

	//hot components cached on add so GetComponent doesn't dynamic_cast scan for them
	//(components are never removed, so these can't go stale)
	TransformComponent* transform = nullptr;
	RigidBodyComponent* rigidBody = nullptr;

public:
	ObjectHandle GetID() const {
		return ID;
	}
	void SetID(ObjectHandle i) {
		ID = i;
	}
	
	template<typename T, typename... Args>
	T* AddComponent(Args&&... args) {
		auto component = std::make_unique<T>(std::forward<Args>(args)...);

		T* ptr = component.get();

		ptr->SetOwner(this);

		components.push_back(std::move(component));

		//first one wins, same as GetComponent's scan order
		if constexpr (std::is_same_v<T, TransformComponent>) {
			if (!transform) transform = ptr;
		}
		else if constexpr (std::is_same_v<T, RigidBodyComponent>) {
			if (!rigidBody) rigidBody = ptr;
		}

		ptr->Start();

		return ptr;
	}
	template<typename T>
	T* GetComponent() {
		if constexpr (std::is_same_v<T, TransformComponent>) return transform;
		else if constexpr (std::is_same_v<T, RigidBodyComponent>) return rigidBody;
		else {
			for (auto& component : components) {
				if (auto cast = dynamic_cast<T*>(component.get()))
					return cast;
			}
			return nullptr;
		}
	}

	template<typename T>
	T* GetComponent(const std::string& s) {
		for (auto& component : components) {
			if (auto cast = dynamic_cast<T*>(component.get()))
				if (component->GetID() == s)
					return cast;
		}
		return nullptr;
	}

	template<typename T>
	std::vector<T*> GetComponents() {
		std::vector<T*> result;
		for (auto& component : components) {
			if (auto cast = dynamic_cast<T*>(component.get()))
				result.push_back(cast);
		}
		return result;
	}

	void Update(float dt) {
		for (auto& component : components) {
			component->Update(dt);
		}
	}
};
