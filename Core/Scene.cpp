#include "Scene.h"
#include "Render/Renderer.h"

GameObject& Scene::CreateObject() {
	std::unique_ptr<GameObject> object = std::make_unique<GameObject>();
	GameObject* ptr = object.get();
	ptr->AddComponent<TransformComponent>();
	objects.push_back(std::move(object));
	dirtyScene = true;
	return *ptr;
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
	physics.Step(dt);
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