#include "Scene.h"
#include "Render/Renderer.h"

GameObject& Scene::CreateObject() {
	std::unique_ptr<GameObject> object = std::make_unique<GameObject>();
	GameObject* ptr = object.get();
	ptr->AddComponent<TransformComponent>();
	objects.push_back(std::move(object));
	return *ptr;
}

void Scene::SetMainCamera(GameObject& camera) {
	mainCamera = &camera;
}

CameraComponent* Scene::GetMainCamera() {
	return mainCamera->GetComponent<CameraComponent>();
}

void Scene::Start() {
	for (std::unique_ptr<GameObject>& object : objects) {
		RigidBodyComponent* rb = object->GetComponent<RigidBodyComponent>();
		ColliderComponent* c = object->GetComponent<ColliderComponent>();
		if (c) physics.colliders.push_back(c);
		if (rb) physics.bodies.push_back(rb);
	}
}
void Scene::Update(float dt) {
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