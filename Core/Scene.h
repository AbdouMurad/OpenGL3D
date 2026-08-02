#pragma once

#include <memory>
#include <vector>

#include "Components/Camera.h"
#include "Components/Lights.h"

#include "Physics/PhysicsWorld.h"

class Renderer;
class RenderLight;
class RenderObject;
class RenderFrame;

class Scene {
	std::vector<std::unique_ptr<GameObject>> objects;
	GameObject* mainCamera = nullptr;
	PhysicsWorld physics;

	bool dirtyScene = false;
public:
	GameObject& CreateObject();

	void SetMainCamera(GameObject& camera);
	CameraComponent* GetMainCamera();

	void Start();
	void Update(float dt);
	void Render(Renderer& renderer);

};