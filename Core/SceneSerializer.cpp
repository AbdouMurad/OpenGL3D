#include "SceneSerializer.h"

void SceneSerializer::Load(const std::string& path, Scene& scene) {
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cout << "FILE COULD NOT BE OPENED: " << path << std::endl;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string jsonText = buffer.str();

	json level = json::parse(jsonText);
	json objects = level["objects"];
	
	for (const json& objectData : objects) {
		std::cout << objectData << std::endl;
		GameObject& obj = scene.CreateObject(objectData["id"]);
		//should always be true
		if (objectData.contains("Transform")) {
			const json& transformData = objectData["Transform"];
			TransformComponent* transform = obj.GetComponent<TransformComponent>();
			if (transformData["parent"] > 0) {
				GameObject& parentObj = scene.GetObject(transformData["parent"]);
				transform->parent = parentObj.GetComponent<TransformComponent>();
			}
			transform->SetPosition(glm::vec3(transformData["position"][0], transformData["position"][1], transformData["position"][2]));
			transform->SetRotation(glm::vec3(transformData["rotation"][0], transformData["rotation"][1], transformData["rotation"][2]));
			transform->SetSize(glm::vec3(transformData["size"][0], transformData["size"][1], transformData["size"][2]));
		}
		if (objectData.contains("Mesh")) {
			obj.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/" + std::string(objectData["Mesh"]["model_name"])));
		}
		if (objectData.contains("RigidBody")) {
			const json& rbData = objectData["RigidBody"];
			RigidBodyComponent* rb = obj.AddComponent<RigidBodyComponent>();
			rb->SetMass(rbData["mass"]);
			if (rbData["static"]) rb->ToggleStatic();
			if (!rbData["gravity"]) rb->ToggleGravity();
			if (rbData.contains("constraints")) {
				for (const std::string& constraint : rbData["constraints"]) {
					if (constraint == "rotation_x")
						rb->AddConstraint(RigidBodyConstraints::FreezeX);
					else if (constraint == "rotation_y")
						rb->AddConstraint(RigidBodyConstraints::FreezeY);
					else if (constraint == "rotation_z")
						rb->AddConstraint(RigidBodyConstraints::FreezeZ);
				}
			}
		}
		if (objectData.contains("Colliders")) {
			for (const json& collider : objectData["Colliders"]) {

				ColliderComponent* c = nullptr;
				if (collider["type"] == "box") {
					c = obj.AddComponent<ColliderComponent>(SHAPE::Box);
					Box* colliderShape = static_cast<Box*>(c->shape.get());
					glm::vec3 extent = { collider["extent"][0], collider["extent"][1], collider["extent"][2] };
					colliderShape->halfExtent = extent / 2.0f;
				}
				else if (collider["type"] == "sphere") {
					c = obj.AddComponent<ColliderComponent>(SHAPE::Sphere);
					Sphere* colliderShape = static_cast<Sphere*>(c->shape.get());
					colliderShape->radius = collider["radius"];
				}
				//TODO: Capsule

				c->localOffset.position = glm::vec3(collider["offset_position"][0], collider["offset_position"][1], collider["offset_position"][2]);
				c->localOffset.rotation = glm::vec3(collider["offset_rotation"][0], collider["offset_rotation"][1], collider["offset_rotation"][2]);
				if (collider["trigger"]) c->isTrigger = true;
				if (collider.contains("id"))
					c->SetID(std::string(collider["id"]));
			}
		}
		if (objectData.contains("PointLight")) {
			const json& lightData = objectData["PointLight"];
			obj.AddComponent<PointLightComponent>(glm::vec3(lightData["color"][0] / 255.0f, lightData["color"][1] / 255.0f, lightData["color"][2] / 255.0f), lightData["intensity"], lightData["range"]);
		}

	}
}