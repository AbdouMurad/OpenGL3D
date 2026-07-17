#pragma once

#include <memory>
#include <vector>

#include "Components/Camera.h"

class Renderer;

class Scene {
	std::vector<std::unique_ptr<GameObject>> objects;

	GameObject* mainCamera = nullptr;

public:
	GameObject& CreateObject();

	void SetMainCamera(GameObject& camera);
	CameraComponent* GetMainCamera();

	void Update(float dt);
	void Render(Renderer& renderer);

};