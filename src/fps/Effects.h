#pragma once

#include "Common.h"
#include <deque>

// Short lived visual effects made of small cubes moved by hand, not by the physics world.
class Effects {
public:
	void Init(Scene& scene, ModelHandle cube);
	void Update(float dt);
	//destroys everything, used on restart
	void Clear();

	void Sparks(const glm::vec3& position, const glm::vec3& normal, int count);
	void Blood(const glm::vec3& position, const glm::vec3& direction, int count);
	void Burst(const glm::vec3& position, const glm::vec3& color, int count, float speed, float size);
	void Tracer(const glm::vec3& from, const glm::vec3& to);
	void BulletHole(const glm::vec3& position, const glm::vec3& normal);

private:
	struct Particle {
		GameObject* object = nullptr;
		glm::vec3 velocity = glm::vec3(0);
		glm::vec3 size = glm::vec3(1);
		float life = 0.0f;
		float maxLife = 0.0f;
		float gravity = 0.0f;
	};
	void Spawn(const glm::vec3& position, const glm::vec3& velocity, const glm::vec3& size,
		const glm::vec3& tint, const glm::vec3& emissive, float life, float gravity,
		const glm::quat& rotation = glm::quat(1, 0, 0, 0));

	static constexpr size_t MaxParticles = 250;
	static constexpr size_t MaxBulletHoles = 80;

	Scene* scene = nullptr;
	ModelHandle cube = 0;
	std::vector<Particle> particles;
	std::deque<GameObject*> bulletHoles;
};
