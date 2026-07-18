#pragma once

#include "GameObject.h"


class PointLightComponent : public Component {
public:
	glm::vec3 color = { 1.0f, 1.0f, 1.0f };
	float intensity = 5.0f;
	float range = 15.0f;
};
