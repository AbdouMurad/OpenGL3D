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

void Scene::Update(float dt) {
	for (std::unique_ptr<GameObject>& object : objects) {
		object->Update(dt);
	}
}

void Scene::Render(Renderer& renderer) {
	mainCamera->GetComponent<CameraComponent>()->updateMatrix();
	renderer.BeginFrame(*GetMainCamera());

	for (std::unique_ptr<GameObject>& object : objects) {
		object->Render(renderer);
	}
}