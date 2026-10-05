#include "Common.h"

#include <random>

static std::mt19937& Rng() {
	static std::mt19937 rng(std::random_device{}());
	return rng;
}

float RandomFloat(float min, float max) {
	return std::uniform_real_distribution<float>(min, max)(Rng());
}
int RandomInt(int min, int max) {
	return std::uniform_int_distribution<int>(min, max)(Rng());
}
glm::vec3 RandomDirection() {
	//uniform height plus uniform angle around it is uniform on a sphere (Archimedes)
	float z = RandomFloat(-1.0f, 1.0f);
	float angle = RandomFloat(0.0f, glm::two_pi<float>());
	float r = std::sqrt(1.0f - z * z);
	return glm::vec3(r * std::cos(angle), r * std::sin(angle), z);
}

float MoveTowards(float current, float target, float maxDelta) {
	if (std::abs(target - current) <= maxDelta) return target;
	return current + (target > current ? maxDelta : -maxDelta);
}
glm::vec3 MoveTowards(const glm::vec3& current, const glm::vec3& target, float maxDelta) {
	glm::vec3 delta = target - current;
	float distance = glm::length(delta);
	if (distance <= maxDelta || distance < 1e-6f) return target;
	return current + delta / distance * maxDelta;
}

glm::quat LookRotation(const glm::vec3& direction) {
	glm::vec3 up = std::abs(direction.y) > 0.99f ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
	return glm::quatLookAt(glm::normalize(direction), up);
}
float YawTowards(const glm::vec3& direction) {
	//rotating (0,0,-1) by yaw around +Y gives (-sin yaw, 0, -cos yaw)
	return std::atan2(-direction.x, -direction.z);
}
float AngleDelta(float from, float to) {
	float delta = std::fmod(to - from + glm::pi<float>(), glm::two_pi<float>());
	if (delta < 0.0f) delta += glm::two_pi<float>();
	return delta - glm::pi<float>();
}

GameObject& CreatePart(Scene& scene, ModelHandle model, TransformComponent* parent,
	const glm::vec3& position, const glm::vec3& size, const glm::vec3& tint, const glm::vec3& emissive) {
	GameObject& part = scene.CreateObject();
	TransformComponent* transform = part.GetComponent<TransformComponent>();
	transform->parent = parent;
	transform->SetPosition(position);
	transform->SetSize(size);

	MeshRenderer* mesh = part.AddComponent<MeshRenderer>(model);
	mesh->tint = tint;
	mesh->emissive = emissive;
	return part;
}

GameObject& CreateStaticBox(Scene& scene, ModelHandle cube, const glm::vec3& position, const glm::vec3& size, const glm::vec3& tint) {
	GameObject& box = CreatePart(scene, cube, nullptr, position, size, tint);
	//no rigid body: the solver treats a plain collider as immovable
	ColliderComponent* collider = box.AddComponent<ColliderComponent>(SHAPE::Box);
	static_cast<Box*>(collider->shape.get())->halfExtent = size * 0.5f;
	return box;
}
