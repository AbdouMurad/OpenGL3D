#include "Player.h"
#include "FPSGame.h"

static constexpr float MouseSensitivity = 0.08f; //degrees per pixel
static constexpr float WalkSpeed = 6.0f;
static constexpr float SprintSpeed = 9.0f;
static constexpr float AimSpeed = 3.4f;
static constexpr float GroundAcceleration = 70.0f;
static constexpr float AirAcceleration = 14.0f;
static constexpr float JumpSpeed = 5.0f;
static constexpr float EyeHeight = 0.72f; //above the body center, ~1.6m off the floor
static constexpr float BaseFov = 64.0f;   //vertical, ~96 degrees horizontal at 16:9

Player::Player(FPSGame* g, GameObject* cam)
	: game(g), camera(cam) {}

void Player::Start() {
	transform = owner->GetComponent<TransformComponent>();
	body = owner->GetComponent<RigidBodyComponent>();
	cameraComponent = camera->GetComponent<CameraComponent>();

	TransformComponent* cameraTransform = camera->GetComponent<TransformComponent>();
	weapons.push_back(std::make_unique<Weapon>(*game, WeaponKind::Rifle, cameraTransform, owner));
	weapons.push_back(std::make_unique<Weapon>(*game, WeaponKind::Shotgun, cameraTransform, owner));
	for (size_t i = 0; i < weapons.size(); ++i) weapons[i]->SetVisible(static_cast<int>(i) == current);
}

void Player::Respawn(const glm::vec3& position, float yawDegrees) {
	transform->SetPosition(position);
	body->SetVelocity(glm::vec3(0.0f));

	health = maxHealth;
	timeSinceDamage = 10.0f;
	damageFlash = 0.0f;
	damageIndicators.clear();
	deathTime = 0.0f;

	yaw = yawDegrees;
	pitch = 0.0f;
	recoil = glm::vec2(0.0f);
	landDip = 0.0f;
	fov = BaseFov;

	current = 0;
	pending = -1;
	for (size_t i = 0; i < weapons.size(); ++i) {
		weapons[i]->Refill();
		weapons[i]->SetVisible(static_cast<int>(i) == current);
	}
	UpdateCamera(0.0f);
}

void Player::SetControlsEnabled(bool enabled) {
	if (enabled && !controlsEnabled) {
		//the click that started or resumed the game shouldn't also fire, and locking the cursor
		//can report one huge mouse jump
		waitForFireRelease = true;
		lookSkipFrames = 2;
	}
	controlsEnabled = enabled;
}

void Player::TakeDamage(float amount, const glm::vec3& sourcePosition) {
	if (IsDead()) return;
	health = std::max(0.0f, health - amount);
	timeSinceDamage = 0.0f;
	damageFlash = std::min(1.0f, damageFlash + amount / 30.0f);

	glm::vec3 toSource = sourcePosition - GetPosition();
	toSource.y = 0.0f;
	if (glm::length2(toSource) > 1e-4f) {
		//yaw grows turning left, the indicator angle grows clockwise on screen
		float angle = -AngleDelta(glm::radians(yaw), YawTowards(toSource));
		damageIndicators.push_back({ angle, 1.0f });
	}
	//getting hit knocks the aim a little
	recoil.x += RandomFloat(-1.0f, 1.0f) * std::min(amount / 8.0f, 2.5f);
	recoil.y += RandomFloat(-1.0f, 1.0f) * std::min(amount / 8.0f, 2.5f);

	if (IsDead()) SetControlsEnabled(false);
}

bool Player::Heal(float amount) {
	if (IsDead() || health >= maxHealth) return false;
	health = std::min(maxHealth, health + amount);
	return true;
}

bool Player::AddAmmo() {
	bool added = false;
	for (auto& weapon : weapons) {
		if (weapon->AddReserve(weapon->Stats().magazineSize)) added = true;
	}
	return added;
}

void Player::EnsureMinimumAmmo() {
	for (auto& weapon : weapons) weapon->EnsureMinimumReserve(weapon->Stats().magazineSize * 2);
}

glm::vec3 Player::GetPosition() const {
	return transform->GetPosition();
}
glm::vec3 Player::GetEyePosition() const {
	return camera->GetComponent<TransformComponent>()->GetPosition();
}
glm::vec3 Player::GetVelocity() const {
	return body->GetVelocity();
}
float Player::GetFov() const {
	return fov;
}

void Player::Update(float dt) {
	timeSinceDamage += dt;
	damageFlash = MoveTowards(damageFlash, 0.0f, dt * 1.5f);
	for (DamageIndicator& indicator : damageIndicators) indicator.timer -= dt * 0.8f;
	std::erase_if(damageIndicators, [](const DamageIndicator& d) { return d.timer <= 0.0f; });
	if (IsDead()) deathTime += dt;

	UpdateLook();
	UpdateMovement(dt);
	//camera first so shots leave from where the crosshair points this frame
	UpdateCamera(dt);
	UpdateWeapons(dt);
}

void Player::UpdateLook() {
	lookDelta = glm::vec2(0.0f);
	if (!controlsEnabled) return;
	if (lookSkipFrames > 0) {
		--lookSkipFrames;
		return;
	}
	lookDelta = Input::GetMouseDelta();
	//scaled with zoom so aiming down sights doesn't feel twitchy
	float sensitivity = MouseSensitivity * (fov / BaseFov);
	yaw = std::fmod(yaw - lookDelta.x * sensitivity, 360.0f);
	pitch = glm::clamp(pitch - lookDelta.y * sensitivity, -89.0f, 89.0f);
}

bool Player::CheckGrounded() const {
	//a few rays down from inside the body, so standing on an edge still counts
	const float inset = Radius - 0.05f;
	const glm::vec3 offsets[5] = {
		{ 0.0f, 0.0f, 0.0f },
		{ inset, 0.0f, inset }, { -inset, 0.0f, inset },
		{ inset, 0.0f, -inset }, { -inset, 0.0f, -inset }
	};
	glm::vec3 center = GetPosition();
	for (const glm::vec3& offset : offsets) {
		RaycastHit hit;
		if (game->scene.Raycast(center + offset, glm::vec3(0, -1, 0), HalfHeight + 0.1f, hit, owner) && hit.normal.y > 0.5f)
			return true;
	}
	return false;
}

void Player::UpdateMovement(float dt) {
	glm::vec3 velocity = body->GetVelocity();

	bool wasGrounded = grounded;
	//still rising from a jump means the rays can see the floor but we aren't on it
	grounded = velocity.y < 2.0f && CheckGrounded();
	if (grounded) {
		coyoteTimer = 0.12f;
		if (!wasGrounded && lastFallSpeed < -4.0f) landDip = std::min(0.15f, -lastFallSpeed * 0.015f);
		lastFallSpeed = 0.0f;
	}
	else {
		coyoteTimer -= dt;
		lastFallSpeed = std::min(lastFallSpeed, velocity.y);
	}

	glm::vec3 wish(0.0f);
	bool aiming = false;
	sprinting = false;
	jumpBufferTimer -= dt;
	if (controlsEnabled) {
		float yawRadians = glm::radians(yaw);
		glm::vec3 forward(-std::sin(yawRadians), 0.0f, -std::cos(yawRadians));
		glm::vec3 right(std::cos(yawRadians), 0.0f, -std::sin(yawRadians));
		if (Input::GetKey(GLFW_KEY_W)) wish += forward;
		if (Input::GetKey(GLFW_KEY_S)) wish -= forward;
		if (Input::GetKey(GLFW_KEY_D)) wish += right;
		if (Input::GetKey(GLFW_KEY_A)) wish -= right;
		if (Input::GetKeyDown(GLFW_KEY_SPACE)) jumpBufferTimer = 0.15f;

		aiming = Input::GetMouseButton(1);
		sprinting = Input::GetKey(GLFW_KEY_LEFT_SHIFT) && Input::GetKey(GLFW_KEY_W) && !aiming && !Input::GetMouseButton(0);
	}

	float speed = sprinting ? SprintSpeed : (aiming ? AimSpeed : WalkSpeed);
	if (glm::length2(wish) > 0.0f) wish = glm::normalize(wish) * speed;

	glm::vec3 horizontal(velocity.x, 0.0f, velocity.z);
	horizontal = MoveTowards(horizontal, wish, (grounded ? GroundAcceleration : AirAcceleration) * dt);
	velocity.x = horizontal.x;
	velocity.z = horizontal.z;

	if (jumpBufferTimer > 0.0f && coyoteTimer > 0.0f) {
		velocity.y = JumpSpeed;
		jumpBufferTimer = 0.0f;
		coyoteTimer = 0.0f;
		grounded = false;
	}
	body->SetVelocity(velocity);
	moveFraction = std::min(glm::length(horizontal) / WalkSpeed, 1.0f);
}

void Player::UpdateCamera(float dt) {
	TransformComponent* cameraTransform = camera->GetComponent<TransformComponent>();

	//head bob, one dip per footstep
	bool walking = grounded && moveFraction > 0.1f;
	bobAmount = Damp(bobAmount, walking ? moveFraction : 0.0f, 8.0f, dt);
	if (walking) bobPhase += dt * (sprinting ? 12.0f : 9.0f);
	float aim = CurrentWeapon().GetAimAmount();
	float bobScale = bobAmount * (1.0f - aim * 0.75f);
	landDip = Damp(landDip, 0.0f, 7.0f, dt);

	glm::vec3 position(
		std::cos(bobPhase * 0.5f) * 0.025f * bobScale,
		EyeHeight + std::sin(bobPhase) * 0.03f * bobScale - landDip,
		0.0f);
	float roll = 0.0f;
	if (IsDead()) {
		//collapse onto the floor
		float t = std::min(deathTime / 0.7f, 1.0f);
		t = t * t * (3.0f - 2.0f * t);
		position.y = glm::mix(EyeHeight, -HalfHeight + 0.25f, t);
		roll = 70.0f * t;
	}
	cameraTransform->SetPosition(position);

	glm::quat rotation =
		glm::angleAxis(glm::radians(yaw + recoil.y), glm::vec3(0, 1, 0)) *
		glm::angleAxis(glm::radians(pitch + recoil.x), glm::vec3(1, 0, 0)) *
		glm::angleAxis(glm::radians(roll), glm::vec3(0, 0, 1));
	cameraTransform->SetRotation(rotation);

	float targetFov = glm::mix(BaseFov, CurrentWeapon().Stats().adsFov, aim) + (sprinting ? 5.0f : 0.0f);
	fov = dt > 0.0f ? Damp(fov, targetFov, 12.0f, dt) : targetFov;
	cameraComponent->FOVdeg = fov;
}

void Player::UpdateWeapons(float dt) {
	WeaponInput input;
	if (controlsEnabled) {
		if (waitForFireRelease && !Input::GetMouseButton(0)) waitForFireRelease = false;
		input.fireHeld = !waitForFireRelease && Input::GetMouseButton(0);
		input.firePressed = !waitForFireRelease && Input::GetMouseButtonDown(0);
		input.aimHeld = Input::GetMouseButton(1);
		input.reloadPressed = Input::GetKeyDown(GLFW_KEY_R);

		int count = static_cast<int>(weapons.size());
		int requested = -1;
		if (Input::GetKeyDown(GLFW_KEY_1)) requested = 0;
		if (Input::GetKeyDown(GLFW_KEY_2)) requested = 1;
		if (Input::GetKeyDown(GLFW_KEY_Q)) requested = (current + 1) % count;
		float scroll = Input::GetScrollDelta();
		if (scroll != 0.0f) requested = (current + (scroll < 0.0f ? 1 : count - 1)) % count;
		if (requested >= 0) pending = requested != current ? requested : -1;
	}
	input.moveFraction = moveFraction;
	input.grounded = grounded;
	input.sprinting = sprinting;
	input.lookDelta = lookDelta;

	//switching: lower the current gun, then swap and raise the new one
	if (IsDead()) {
		weapons[current]->SetLowered(true);
	}
	else if (pending >= 0) {
		weapons[current]->SetLowered(true);
		if (weapons[current]->IsFullyLowered()) {
			weapons[current]->SetVisible(false);
			current = pending;
			pending = -1;
			weapons[current]->SetVisible(true);
			weapons[current]->Raise();
		}
	}
	else {
		weapons[current]->SetLowered(false);
	}

	glm::vec2 kick = weapons[current]->Update(dt, input);
	//most of the kick stays and has to be pulled back down, the rest springs back by itself
	pitch = glm::clamp(pitch + kick.x * 0.65f, -89.0f, 89.0f);
	yaw += kick.y * 0.65f;
	recoil += kick * 0.35f;
	recoil = Damp(recoil, glm::vec2(0.0f), 10.0f, dt);
}
