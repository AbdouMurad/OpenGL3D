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
