#pragma once

#include <memory>

#include "GameObject.h"
#include "Render/GrassField.h"
#include "Physics/RigidBody.h"
#include "Core/Time_.h"

//a field of grass. the object's own transform is ignored: GrassSettings says where it grows
//pusher (optional): grass bends away from that object, and where it walks the grass is pressed flat
//in the direction it was going, standing back up over a few seconds (a trail)
class GrassComponent : public Component {
	std::unique_ptr<GrassField> field;
	glm::vec2 lastStamp = glm::vec2(0.0f);
	bool hasLastStamp = false;
public:
	GameObject* pusher = nullptr;
	float pushRadius = 1.1f;  //live push around the pusher (bends outward)
	float trailRadius = 0.5f; //width of the pressed path / 2
	float minTrailSpeed = 0.5f; //slower than this (m/s) leaves no new trail: standing still has no direction

	//builds the GPU buffers right away -> only create while the window (OpenGL context) exists
	GrassComponent(const GrassSettings& settings, GameObject* pusher = nullptr)
		:	field(std::make_unique<GrassField>(settings)), pusher(pusher) {}

	GrassField& GetField() { return *field; }

	void Update(float dt) override {
		if (!pusher) return;
		glm::vec3 position = pusher->GetComponent<TransformComponent>()->GetPosition();
		glm::vec2 here(position.x, position.z);

		//only while on (or right above) the ground: jumping over grass doesn't flatten it
		bool touching = position.y - field->GetGroundHeight() < 0.9f;
		RigidBodyComponent* rb = pusher->GetComponent<RigidBodyComponent>();
		glm::vec2 velocity = rb ? glm::vec2(rb->GetVelocity().x, rb->GetVelocity().z) : glm::vec2(0.0f);
		float speed = glm::length(velocity);

		if (!touching || speed < minTrailSpeed) {
			hasLastStamp = false;
			return;
		}
		glm::vec2 direction = velocity / speed;
		float now = Time::TotalTime();

		//stamp every half radius along the way since last frame -> no gaps when moving fast (dash) or at low fps
		glm::vec2 from = hasLastStamp ? lastStamp : here;
		float travelled = glm::length(here - from);
		float spacing = trailRadius * 0.5f;
		int steps = std::min(64, static_cast<int>(travelled / spacing));
		for (int i = 1; i <= steps; ++i) {
			field->Stamp(glm::mix(from, here, i * spacing / travelled), direction, trailRadius, now);
		}
		field->Stamp(here, direction, trailRadius, now);
		lastStamp = here;
		hasLastStamp = true;
	}
};
