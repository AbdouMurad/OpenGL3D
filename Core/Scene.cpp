#include "Scene.h"
#include "Render/DebugRender.h"
#include <cmath>
#include <unordered_set>

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

void Scene::Destroy(GameObject& object) {
	pendingDestroy.push_back(&object);
}

void Scene::FlushDestroyed() {
	if (pendingDestroy.empty()) return;

	std::unordered_set<GameObject*> doomed(pendingDestroy.begin(), pendingDestroy.end());
	pendingDestroy.clear();

	//children go with their parent, repeat until a pass adds nothing so grandchildren are caught too
	bool added = true;
	while (added) {
		added = false;
		for (std::unique_ptr<GameObject>& object : objects) {
			if (doomed.contains(object.get())) continue;
			TransformComponent* parent = object->GetComponent<TransformComponent>()->parent;
			if (parent && doomed.contains(parent->getOwner())) {
				doomed.insert(object.get());
				added = true;
			}
		}
	}

	for (GameObject* object : doomed) {
		physics.RemoveObject(*object);
		for (Component* component : object->GetComponents<Component>()) {
			eventBus.Unsubscribe(component);
		}
	}
	std::erase_if(objects, [&doomed](const std::unique_ptr<GameObject>& object) {
		return doomed.contains(object.get());
	});
}

bool Scene::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit& hit, const GameObject* ignore) {
	//objects created this frame aren't in the physics lists yet
	if (dirtyScene) Start();
	return physics.Raycast(origin, direction, maxDistance, hit, ignore);
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
	//by index: an update may create objects, which can reallocate the vector
	for (size_t i = 0; i < objects.size(); ++i) {
		objects[i]->Update(dt);
	}

	const float maxPhysicsStep = 1.0f / 120.0f;
	int subSteps = static_cast<int>(std::ceil(dt / maxPhysicsStep));
	if (subSteps < 1) subSteps = 1;
	if (subSteps > 8) subSteps = 8;

	const float stepDt = dt / static_cast<float>(subSteps);
	for (int i = 0; i < subSteps; ++i) {
		physics.Step(stepDt);
	}
	FlushDestroyed();
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
				renderer.DrawBox(mat[3], axes, shape->halfExtent);
			}
			if (collider->shape.get()->GetType() == SHAPE::Sphere) {
				glm::mat4 mat = collider->GetMatrix();
				renderer.DrawSphere(mat[3], static_cast<Sphere*>(collider->shape.get())->radius);
			}
		}
	}
	physics.DebugRender(renderer);
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
		if (mesh && mesh->visible) {
			frame.renderObjects.push_back({
				transform->GetMatrix(),
				mesh->modelID,
				mesh->tint,
				mesh->emissive,
				mesh->overlay
			});
		}
	}
	renderer.Render(frame);
}