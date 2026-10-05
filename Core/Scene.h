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

	std::vector<GameObject*> pendingDestroy;
	void FlushDestroyed();
public:
	EventBus eventBus;
	GameObject& GetObject(ObjectHandle id);
	GameObject& CreateObject();
	GameObject& CreateObject(ObjectHandle i);

	//deleted at the end of Update, along with every object parented to it. Pointers to it stay valid until then
	void Destroy(GameObject& object);

	bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit& hit, const GameObject* ignore = nullptr);

	void SetMainCamera(GameObject& camera);
	CameraComponent* GetMainCamera();

	void Start();
	void Update(float dt);
	void Render(Renderer& renderer);
	void DebugRender(DebugRenderer& renderer);

};