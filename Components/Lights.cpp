#include "Lights.h"

PointLightComponent::PointLightComponent(glm::vec3 color, float intensity, float range)
	:	color(color), intensity(intensity), range(range) {}

DirectionalLightComponent::DirectionalLightComponent(glm::vec3 color, float intensity)
	:	color(color), intensity(intensity) {}