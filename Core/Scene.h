#pragma once

#include <memory>
#include <vector>

#include "Components/Camera.h"
#include "Components/Lights.h"

#include "Physics/PhysicsWorld.h"
#include <stdexcept>

class Renderer;
class DebugRenderer;
class RenderLight;
class RenderObject;
class RenderFrame;

class Scene {
	std::vector<std::unique_ptr<GameObject>> objects;
	GameObject* mainCamera = nullptr;
	PhysicsWorld physics = PhysicsWorld(eventBus);

	bool dirtyScene = false;
	ObjectHandle nextID = 1;
	float physicsAccumulator = 0.0f; //frame time not yet simulated (fixed timestep)
public:
	EventBus eventBus;
	GameObject& GetObject(ObjectHandle id);
	GameObject& CreateObject();
	GameObject& CreateObject(ObjectHandle i);

	void SetMainCamera(GameObject& camera);
	CameraComponent* GetMainCamera();

	void Start();
	void Clear();
	void Update(float dt);
	void Render(Renderer& renderer);
	void DebugRender(DebugRenderer& renderer);

	//closest collider along a ray (see RaycastHit / RaycastOptions in PhysicsWorld.h). false = hit nothing
	bool Raycast(const glm::vec3& origin, const glm::vec3& direction, RaycastHit& hit, const RaycastOptions& options = {});

};