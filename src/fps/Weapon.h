#pragma once

#include "Common.h"

class FPSGame;

enum class WeaponKind { Rifle, Shotgun };

struct WeaponStats {
	const char* name;
	float damage;             //per pellet
	float headshotMultiplier;
	int pellets;
	float fireInterval;       //seconds between shots
	bool automatic;
	int magazineSize;
	int startReserve;
	int maxReserve;
	float reloadTime;         //whole magazine, or per shell when reloadPerShell
	bool reloadPerShell;
	float hipSpread;          //cone half angle in degrees
	float adsSpread;
	float moveSpread;         //added when moving at full speed
	float bloomPerShot;       //added per shot, recovers when not firing
	float maxBloom;
	float recoilPitch;        //view kick per shot in degrees
	float recoilYaw;
	float range;
	float impulse;            //push given to physics objects per pellet
	float adsFov;             //vertical, like CameraComponent::FOVdeg
	glm::vec3 hipPosition;    //viewmodel position relative to the camera
	glm::vec3 adsPosition;
	glm::vec3 muzzleOffset;   //barrel tip relative to the viewmodel
};

struct WeaponInput {
	bool fireHeld = false;
	bool firePressed = false;
	bool aimHeld = false;
	bool reloadPressed = false;
	float moveFraction = 0.0f; //0 standing still, 1 at full run speed
	bool grounded = true;
	bool sprinting = false;
	glm::vec2 lookDelta = glm::vec2(0);
};

// A first person gun: ammo, firing and the viewmodel, which is built from cubes parented to the camera.
class Weapon {
public:
	Weapon(FPSGame& game, WeaponKind kind, TransformComponent* camera, GameObject* shooter);

	//returns view recoil to apply this frame in degrees, x = pitch, y = yaw
	glm::vec2 Update(float dt, const WeaponInput& input);

	void SetVisible(bool visible);
	//lowered weapons can't fire, used when switching
	void SetLowered(bool lowered);
	//starts fully lowered and comes up, for the weapon being switched to
	void Raise();
	bool IsFullyLowered() const { return lowerAmount >= 1.0f; }

	void Refill();
	//tops up the reserve, returns false when it was already full
	bool AddReserve(int amount);
	void EnsureMinimumReserve(int amount);

	const WeaponStats& Stats() const { return stats; }
	int GetAmmo() const { return ammo; }
	int GetReserve() const { return reserve; }
	bool IsReloading() const { return reloading; }
	float GetReloadProgress() const;
	float GetSpread() const;                  //current cone half angle in degrees
	float GetAimAmount() const { return aimAmount; } //0 hip fire, 1 aiming down sights

private:
	void BuildRifle();
	void BuildShotgun();
	GameObject& AddPart(const glm::vec3& position, const glm::vec3& size, const glm::vec3& tint,
		const glm::vec3& emissive = glm::vec3(0), const glm::vec3& rotation = glm::vec3(0));
	void Fire();
	void StartReload();
	void UpdateViewmodel(float dt, const WeaponInput& input);

	FPSGame& game;
	WeaponKind kind;
	WeaponStats stats;
	TransformComponent* camera = nullptr;
	GameObject* shooter = nullptr;

	TransformComponent* root = nullptr;     //everything else is parented to this
	TransformComponent* muzzle = nullptr;
	TransformComponent* magazine = nullptr; //rifle magazine or shotgun pump, animated
	glm::vec3 magazineRest = glm::vec3(0);
	MeshRenderer* flash = nullptr;
	PointLightComponent* flashLight = nullptr;
	std::vector<MeshRenderer*> meshes;

	int ammo = 0;
	int reserve = 0;
	float fireTimer = 0.0f;
	float bloom = 0.0f;
	float timeSinceShot = 10.0f;
	bool reloading = false;
	float reloadTimer = 0.0f;
	float flashTimer = 0.0f;
	float moveFraction = 0.0f;

	float aimAmount = 0.0f;
	float lowerAmount = 0.0f;
	bool lowered = false;
	float kick = 0.0f;
	glm::vec2 sway = glm::vec2(0);
	float bobPhase = 0.0f;
	float bobAmount = 0.0f;
};
