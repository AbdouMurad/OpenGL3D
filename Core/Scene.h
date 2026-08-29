#pragma once

#include <memory>
#include <vector>

#include "Components/Camera.h"
#include "Components/Lights.h"

#include "Physics/PhysicsWorld.h"
#include <stdexcept>

class Renderer;
class RenderLight;
class RenderObject;
class RenderFrame;

class Scene {
	std::vector<std::unique_ptr<GameObject>> objects;
	GameObject* mainCamera = nullptr;
	PhysicsWorld physics = PhysicsWorld(eventBus);

	bool dirtyScene = false;
	ObjectHandle nextID = 1;
public:
	EventBus eventBus;
	GameObject& GetObject(ObjectHandle id);
	GameObject& CreateObject();
	GameObject& CreateObject(ObjectHandle i);

	void SetMainCamera(GameObject& camera);
	CameraComponent* GetMainCamera();

	void Start();
	void Update(float dt);
	void Render(Renderer& renderer);

};