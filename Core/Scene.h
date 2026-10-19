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

//how the whole world looks, set by the game in Start(). reset by Clear()
struct Environment {
	glm::vec3 skyColor = glm::vec3(0.0f); //background, also what fog fades into
	float fogDensity = 0.0f;			  //0 = no fog. ~0.03 = gone at about 60 m
	bool grayWorld = false;				  //everything grayscale except where point lights reach
};

class Scene {
	std::vector<std::unique_ptr<GameObject>> objects;
	GameObject* mainCamera = nullptr;
	PhysicsWorld physics = PhysicsWorld(eventBus);

	bool dirtyScene = false;
	ObjectHandle nextID = 1;
	float physicsAccumulator = 0.0f; //frame time not yet simulated (fixed timestep)
public:
	EventBus eventBus;
	Environment environment;
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