#pragma once

#include "Core/Engine.h"

float RandomFloat(float min, float max);
int RandomInt(int min, int max); //inclusive
glm::vec3 RandomDirection();     //uniform over the unit sphere

float MoveTowards(float current, float target, float maxDelta);
glm::vec3 MoveTowards(const glm::vec3& current, const glm::vec3& target, float maxDelta);
//frame rate independent exponential smoothing towards target, higher rate gets there faster
template<typename T>
T Damp(const T& current, const T& target, float rate, float dt) {
	return glm::mix(current, target, 1.0f - std::exp(-rate * dt));
}

//rotation that points the -Z axis (TransformComponent::Forward) along direction
glm::quat LookRotation(const glm::vec3& direction);
//yaw in radians around +Y that faces a flat direction, 0 faces -Z
float YawTowards(const glm::vec3& direction);
//shortest signed difference between two angles in radians
float AngleDelta(float from, float to);

//an object with a mesh and nothing else, the building block for the arena, enemies and weapon models
GameObject& CreatePart(Scene& scene, ModelHandle model, TransformComponent* parent,
	const glm::vec3& position, const glm::vec3& size,
	const glm::vec3& tint = glm::vec3(1.0f), const glm::vec3& emissive = glm::vec3(0.0f));

//a box that blocks movement and bullets but never moves
GameObject& CreateStaticBox(Scene& scene, ModelHandle cube, const glm::vec3& position, const glm::vec3& size, const glm::vec3& tint);
