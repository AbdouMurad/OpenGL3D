#pragma once

#include "Physics/Collider.h"

struct TriggerEnterEvent {
	ColliderComponent* trigger = nullptr;
	ColliderComponent* other = nullptr;
};

struct TriggerExitEvent {
	ColliderComponent* trigger = nullptr;
	ColliderComponent* other = nullptr;
};