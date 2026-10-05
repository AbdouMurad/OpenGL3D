#pragma once

#include "Weapon.h"

class FPSGame;

// First person controller. The body is an upright box that never rotates; looking around only turns the
// camera, a child of the body, so the collider can't be twisted into walls.
class Player : public Component {
public:
	Player(FPSGame* game, GameObject* camera);
	void Start() override;
	void Update(float dt) override;

	void Respawn(const glm::vec3& position, float yawDegrees);
	void TakeDamage(float amount, const glm::vec3& sourcePosition);
	bool Heal(float amount); //false when already at full health
	bool AddAmmo();          //a magazine's worth for each weapon, false when all were full
	void EnsureMinimumAmmo();

	//the game turns this off on menus, while paused and when dead
	void SetControlsEnabled(bool enabled);

	bool IsDead() const { return health <= 0.0f; }
	float GetHealth() const { return health; }
	float GetMaxHealth() const { return maxHealth; }
	glm::vec3 GetPosition() const;    //center of the body
	glm::vec3 GetEyePosition() const;
	glm::vec3 GetVelocity() const;
	float GetFov() const;
	Weapon& CurrentWeapon() { return *weapons[current]; }

	//HUD feedback
	float damageFlash = 0.0f;
	struct DamageIndicator {
		float angle; //radians clockwise from straight ahead
		float timer;
	};
	std::vector<DamageIndicator> damageIndicators;

	static constexpr float HalfHeight = 0.9f;
	static constexpr float Radius = 0.35f;

private:
	bool CheckGrounded() const;
	void UpdateLook();
	void UpdateMovement(float dt);
	void UpdateWeapons(float dt);
	void UpdateCamera(float dt);

	FPSGame* game = nullptr;
	GameObject* camera = nullptr;
	CameraComponent* cameraComponent = nullptr;
	TransformComponent* transform = nullptr;
	RigidBodyComponent* body = nullptr;

	std::vector<std::unique_ptr<Weapon>> weapons;
	int current = 0;
	int pending = -1;

	bool controlsEnabled = false;
	bool waitForFireRelease = true;
	int lookSkipFrames = 0;

	float health = 100.0f;
	float maxHealth = 100.0f;
	float timeSinceDamage = 10.0f;

	float yaw = 0.0f;   //degrees
	float pitch = 0.0f;
	glm::vec2 recoil = glm::vec2(0); //added on top of the aim, recovers back to it
	glm::vec2 lookDelta = glm::vec2(0);

	bool grounded = false;
	float coyoteTimer = 0.0f;
	float jumpBufferTimer = 0.0f;
	float lastFallSpeed = 0.0f;
	bool sprinting = false;
	float moveFraction = 0.0f;

	float bobPhase = 0.0f;
	float bobAmount = 0.0f;
	float landDip = 0.0f;
	float deathTime = 0.0f;
	float fov = 72.0f;
};
