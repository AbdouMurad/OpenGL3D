#pragma once

#include "GameObject.h"


class PointLightComponent : public Component {
public:
	PointLightComponent(glm::vec3 color, float intensity, float range);
	PointLightComponent() = default;

	glm::vec3 color = { 1.0f, 1.0f, 1.0f };
	float intensity = 1.0f;
	float range = 15.0f;
};

//sun-like light: same direction everywhere and no falloff with distance
//shines along the owner's forward (-z turned by its rotation), so rotate the object to aim it
class DirectionalLightComponent : public Component {
public:
	DirectionalLightComponent(glm::vec3 color, float intensity);
	DirectionalLightComponent() = default;

	glm::vec3 color = { 1.0f, 1.0f, 1.0f };
	float intensity = 1.0f;
};
