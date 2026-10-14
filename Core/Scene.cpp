#include "Scene.h"
#include "Render/DebugRender.h"
#include <cmath>
#include <algorithm>

GameObject& Scene::CreateObject() {
	std::unique_ptr<GameObject> object = std::make_unique<GameObject>();
	GameObject* ptr = object.get();
	ptr->AddComponent<TransformComponent>();
	ptr->SetID(nextID++);
	objects.push_back(std::move(object));
	dirtyScene = true;
	return *ptr;
}
GameObject& Scene::CreateObject(ObjectHandle i) {
	std::unique_ptr<GameObject> object = std::make_unique<GameObject>();
	GameObject* ptr = object.get();
	ptr->SetID(i);
	nextID = i + 1;
	ptr->AddComponent<TransformComponent>();
	objects.push_back(std::move(object));
	dirtyScene = true;
	return *ptr;
}
GameObject& Scene::GetObject(ObjectHandle id) {
	for (std::unique_ptr<GameObject>& ptr : objects) {
		if (ptr->GetID() == id) return *ptr.get();
	}
	throw std::out_of_range("Game Object ID not found");
}

void Scene::SetMainCamera(GameObject& camera) {
	mainCamera = &camera;
}

CameraComponent* Scene::GetMainCamera() {
	return mainCamera->GetComponent<CameraComponent>();
}

void Scene::Start() {
	physics.colliders.clear();
	physics.bodies.clear();
	for (std::unique_ptr<GameObject>& object : objects) {
		for (RigidBodyComponent* rb : object->GetComponents<RigidBodyComponent>()) {
			physics.bodies.push_back(rb);
		}
		for (ColliderComponent* c : object->GetComponents<ColliderComponent>()) {
			physics.colliders.push_back(c);
		}
	}
	dirtyScene = false;
}
//Removes everything so the scene can be rebuilt from scratch (physics first, it holds pointers into the objects)
void Scene::Clear() {
	physics.Clear();
	eventBus.Clear();
	objects.clear();
	mainCamera = nullptr;
	nextID = 1;
	dirtyScene = false;
	physicsAccumulator = 0.0f;
}
void Scene::Update(float dt) {
	if (dirtyScene) Start();
	for (std::unique_ptr<GameObject>& object : objects) {
		object->Update(dt);
	}

	//fixed timestep: physics always advances in exact 1/120 s steps, whatever the frame rate
	//-> damping, sleeping and position correction behave the same at 60 fps and 1000 fps
	//frame time left over (less than a step) carries over to the next frame
	constexpr float physicsStep = 1.0f / 120.0f;
	//cap -> a slow frame can't request more steps, which would make the next frame slower (spiral)
	constexpr int maxStepsPerFrame = 4;

	physicsAccumulator += dt;
	int steps = 0;
	while (physicsAccumulator >= physicsStep && steps < maxStepsPerFrame) {
		physics.Step(physicsStep);
		physicsAccumulator -= physicsStep;
		++steps;
	}
	//still behind after the cap -> drop the backlog, physics runs slower than real time instead of catching up
	if (physicsAccumulator >= physicsStep) physicsAccumulator = 0.0f;
}

bool Scene::Raycast(const glm::vec3& origin, const glm::vec3& direction, RaycastHit& hit, const RaycastOptions& options) {
	if (dirtyScene) Start(); //objects added this frame aren't in the physics lists until the next Start()
	return physics.Raycast(origin, direction, hit, options);
}

void Scene::DebugRender(DebugRenderer& renderer) {
	for (const auto& obj : objects) {
		ColliderComponent* collider = obj.get()->GetComponent<ColliderComponent>();
		if (collider) {
			if (collider->shape.get()->GetType() == SHAPE::Box) {
				glm::mat4 mat = collider->GetMatrix();
				Box* shape = static_cast<Box*>(collider->shape.get());
				glm::vec3 axes[3] = {
					glm::normalize(mat[0]),
					glm::normalize(mat[1]),
					glm::normalize(mat[2])
				};
				renderer.DrawBox(mat[3], axes, shape->GetHalfExtent());
			}
			if (collider->shape.get()->GetType() == SHAPE::Sphere) {
				glm::mat4 mat = collider->GetMatrix();
				renderer.DrawSphere(mat[3], static_cast<Sphere*>(collider->shape.get())->GetRadius());
			}
		}
	}
	//physics.DebugRender(renderer);
	renderer.Render(GetMainCamera()->cameraMatrix);
}

void Scene::Render(Renderer& renderer) {
	
	//renderer.BeginFrame(); Not needed rn
	RenderFrame frame;
	GetMainCamera()->UpdateMatrix();
	frame.cameraMatrix = GetMainCamera()->cameraMatrix;
	frame.cameraPosition = GetMainCamera()->GetPosition();
	for (std::unique_ptr<GameObject>& object : objects) {
		auto* light = object.get()->GetComponent<PointLightComponent>();
		auto* transform = object.get()->GetComponent<TransformComponent>();
		auto* mesh = object.get()->GetComponent<MeshRenderer>();
		if (!transform) continue;
		if (light)
			frame.lights.push_back({
				transform->GetPosition(),
				light->color, //write getters and setters
				light->intensity,
				light->range
			});
		if (mesh) {
			frame.renderObjects.push_back({
				transform->GetMatrix(),
				mesh->modelID
			});
		}
	}
	renderer.Render(frame);
}