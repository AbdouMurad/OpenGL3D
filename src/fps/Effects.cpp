#include "Effects.h"

void Effects::Init(Scene& s, ModelHandle c) {
	scene = &s;
	cube = c;
}

void Effects::Spawn(const glm::vec3& position, const glm::vec3& velocity, const glm::vec3& size,
	const glm::vec3& tint, const glm::vec3& emissive, float life, float gravity, const glm::quat& rotation) {
	if (particles.size() >= MaxParticles) return;

	GameObject& object = CreatePart(*scene, cube, nullptr, position, size, tint, emissive);
	object.GetComponent<TransformComponent>()->SetRotation(rotation);
	particles.push_back({ &object, velocity, size, life, life, gravity });
}

void Effects::Update(float dt) {
	for (size_t i = 0; i < particles.size();) {
		Particle& p = particles[i];
		p.life -= dt;
		if (p.life <= 0.0f) {
			scene->Destroy(*p.object);
			particles[i] = particles.back();
			particles.pop_back();
			continue;
		}

		TransformComponent* transform = p.object->GetComponent<TransformComponent>();
		p.velocity.y -= p.gravity * dt;
		glm::vec3 position = transform->GetLocalPosition() + p.velocity * dt;
		//cheap bounce off the arena floor, nothing else is solid to particles
		if (position.y < 0.02f && p.gravity > 0.0f) {
			position.y = 0.02f;
			p.velocity.y *= -0.3f;
			p.velocity.x *= 0.6f;
			p.velocity.z *= 0.6f;
		}
		transform->SetPosition(position);
		transform->SetSize(p.size * (p.life / p.maxLife));
		++i;
	}
}

void Effects::Clear() {
	for (Particle& p : particles) scene->Destroy(*p.object);
	particles.clear();
	for (GameObject* hole : bulletHoles) scene->Destroy(*hole);
	bulletHoles.clear();
}

void Effects::Sparks(const glm::vec3& position, const glm::vec3& normal, int count) {
	for (int i = 0; i < count; ++i) {
		//mostly out of the surface, scattered
		glm::vec3 direction = glm::normalize(normal * 1.2f + RandomDirection());
		float size = RandomFloat(0.025f, 0.05f);
		Spawn(position + normal * 0.03f, direction * RandomFloat(2.5f, 6.0f), glm::vec3(size),
			glm::vec3(1.0f, 0.8f, 0.4f), glm::vec3(4.0f, 2.2f, 0.6f), RandomFloat(0.15f, 0.35f), 9.0f);
	}
	//a puff of dust that hangs in the air a little longer
	Spawn(position + normal * 0.05f, normal * 0.4f, glm::vec3(0.12f), glm::vec3(0.5f, 0.48f, 0.45f), glm::vec3(0.0f), 0.35f, 0.0f);
}

void Effects::Blood(const glm::vec3& position, const glm::vec3& direction, int count) {
	for (int i = 0; i < count; ++i) {
		//robots leak oil and sparks rather than blood: dark chunks with the odd hot fleck
		glm::vec3 velocity = glm::normalize(direction + RandomDirection() * 0.8f) * RandomFloat(1.5f, 5.0f);
		float size = RandomFloat(0.04f, 0.09f);
		bool hot = i % 3 == 0;
		Spawn(position, velocity, glm::vec3(size),
			hot ? glm::vec3(1.0f, 0.6f, 0.2f) : glm::vec3(0.12f, 0.1f, 0.1f),
			hot ? glm::vec3(3.0f, 1.2f, 0.2f) : glm::vec3(0.0f),
			RandomFloat(0.4f, 0.9f), 12.0f);
	}
}

void Effects::Burst(const glm::vec3& position, const glm::vec3& color, int count, float speed, float size) {
	for (int i = 0; i < count; ++i) {
		glm::vec3 direction = RandomDirection();
		direction.y = std::abs(direction.y);
		float s = size * RandomFloat(0.6f, 1.3f);
		Spawn(position, direction * speed * RandomFloat(0.4f, 1.0f), glm::vec3(s), color, color * 2.5f,
			RandomFloat(0.35f, 0.8f), 6.0f, glm::quat(RandomDirection() * glm::pi<float>()));
	}
}

void Effects::Tracer(const glm::vec3& from, const glm::vec3& to) {
	glm::vec3 delta = to - from;
	float length = glm::length(delta);
	if (length < 0.5f) return;
	//a thin streak covering the first part of the path, it fades within a couple of frames
	float streak = std::min(length, 12.0f);
	glm::vec3 direction = delta / length;
	glm::vec3 center = from + direction * (streak * 0.5f + 0.3f);
	Spawn(center, direction * 60.0f, glm::vec3(0.018f, 0.018f, streak), glm::vec3(1.0f, 0.9f, 0.6f),
		glm::vec3(3.0f, 2.4f, 1.2f), 0.05f, 0.0f, LookRotation(direction));
}

void Effects::BulletHole(const glm::vec3& position, const glm::vec3& normal) {
	if (bulletHoles.size() >= MaxBulletHoles) {
		scene->Destroy(*bulletHoles.front());
		bulletHoles.pop_front();
	}
	float size = RandomFloat(0.07f, 0.1f);
	//sits just off the surface so it doesn't z-fight with it
	GameObject& hole = CreatePart(*scene, cube, nullptr, position + normal * 0.004f, glm::vec3(size, size, 0.01f),
		glm::vec3(0.02f, 0.02f, 0.02f));
	hole.GetComponent<TransformComponent>()->SetRotation(LookRotation(normal) * glm::angleAxis(RandomFloat(0.0f, 1.5f), glm::vec3(0, 0, 1)));
	bulletHoles.push_back(&hole);
}
