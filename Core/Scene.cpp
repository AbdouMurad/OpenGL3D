#include "Scene.h"
#include "Render/Renderer.h"
#include <cmath>

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
void Scene::Update(float dt) {
	if (dirtyScene) Start();
	for (std::unique_ptr<GameObject>& object : objects) {
		object->Update(dt);
	}

	const float maxPhysicsStep = 1.0f / 120.0f;
	int subSteps = static_cast<int>(std::ceil(dt / maxPhysicsStep));
	if (subSteps < 1) subSteps = 1;
	if (subSteps > 8) subSteps = 8;

	const float stepDt = dt / static_cast<float>(subSteps);
	for (int i = 0; i < subSteps; ++i) {
		physics.Step(stepDt);
	}
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