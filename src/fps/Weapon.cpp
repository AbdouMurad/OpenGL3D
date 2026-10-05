#include "Weapon.h"
#include "FPSGame.h"

static const WeaponStats RifleStats = {
	"ASSAULT RIFLE",
	24.0f, 2.0f, 1,           //damage, headshot multiplier, pellets
	0.095f, true,             //~630 rounds per minute, automatic
	30, 120, 240,             //magazine, starting and max reserve
	1.7f, false,              //magazine reload
	1.0f, 0.12f, 2.2f,        //hip, aimed and moving spread
	0.3f, 3.0f,               //bloom
	0.42f, 0.22f,             //recoil
	150.0f, 4.0f, 44.0f,      //range, impulse, aimed fov
	{ 0.2f, -0.2f, -0.45f },
	{ 0.0f, -0.08f, -0.32f },
	{ 0.0f, 0.015f, -0.56f },
};

static const WeaponStats ShotgunStats = {
	"SHOTGUN",
	13.0f, 1.5f, 9,
	0.8f, true,               //pump action, holding the trigger fires as soon as it's racked
	6, 24, 48,
	0.42f, true,              //shell by shell
	4.5f, 3.2f, 1.0f,
	0.0f, 0.0f,
	3.6f, 0.9f,
	60.0f, 6.0f, 54.0f,
	{ 0.21f, -0.21f, -0.5f },
	{ 0.0f, -0.062f, -0.38f },
	{ 0.0f, 0.025f, -0.72f },
};

Weapon::Weapon(FPSGame& g, WeaponKind k, TransformComponent* cameraTransform, GameObject* shooterObject)
	: game(g), kind(k), stats(k == WeaponKind::Rifle ? RifleStats : ShotgunStats), camera(cameraTransform), shooter(shooterObject) {

	GameObject& rootObject = game.scene.CreateObject();
	root = rootObject.GetComponent<TransformComponent>();
	root->parent = camera;
	root->SetPosition(stats.hipPosition);

	if (kind == WeaponKind::Rifle) BuildRifle();
	else BuildShotgun();

	GameObject& muzzleObject = game.scene.CreateObject();
	muzzle = muzzleObject.GetComponent<TransformComponent>();
	muzzle->parent = root;
	muzzle->SetPosition(stats.muzzleOffset);
	flashLight = muzzleObject.AddComponent<PointLightComponent>(glm::vec3(1.0f, 0.75f, 0.4f), 0.0f, 9.0f);

	GameObject& flashObject = CreatePart(game.scene, game.sphereModel, muzzle, glm::vec3(0), glm::vec3(0.05f), glm::vec3(1.0f), glm::vec3(2.2f, 0.9f, 0.15f));
	flash = flashObject.GetComponent<MeshRenderer>();
	flash->overlay = true;
	flash->visible = false;

	ammo = stats.magazineSize;
	reserve = stats.startReserve;
}

GameObject& Weapon::AddPart(const glm::vec3& position, const glm::vec3& size, const glm::vec3& tint, const glm::vec3& emissive, const glm::vec3& rotation) {
	GameObject& part = CreatePart(game.scene, game.cubeModel, root, position, size, tint, emissive);
	part.GetComponent<TransformComponent>()->SetRotation(rotation);
	MeshRenderer* mesh = part.GetComponent<MeshRenderer>();
	mesh->overlay = true;
	meshes.push_back(mesh);
	return part;
}

void Weapon::BuildRifle() {
	const glm::vec3 body(0.16f, 0.16f, 0.18f);
	const glm::vec3 black(0.05f, 0.05f, 0.06f);
	const glm::vec3 olive(0.24f, 0.26f, 0.18f);
	const glm::vec3 glow(3.0f, 1.3f, 0.2f);

	AddPart({ 0.0f, 0.0f, 0.0f }, { 0.07f, 0.09f, 0.42f }, body);             //receiver
	AddPart({ 0.0f, 0.015f, -0.37f }, { 0.03f, 0.03f, 0.34f }, black);        //barrel
	AddPart({ 0.0f, -0.003f, -0.27f }, { 0.078f, 0.072f, 0.2f }, olive);      //handguard
	AddPart({ 0.0f, -0.09f, 0.1f }, { 0.045f, 0.12f, 0.05f }, black, glm::vec3(0), { -18.0f, 0.0f, 0.0f }); //grip
	AddPart({ 0.0f, -0.012f, 0.29f }, { 0.055f, 0.08f, 0.2f }, olive);        //stock
	//iron sights: a notch at the back to look through, lined up with the front post at the crosshair
	AddPart({ 0.0f, 0.0525f, 0.13f }, { 0.04f, 0.015f, 0.03f }, black);       //rear sight base
	AddPart({ -0.012f, 0.07f, 0.13f }, { 0.008f, 0.02f, 0.012f }, black);     //rear sight ears
	AddPart({ 0.012f, 0.07f, 0.13f }, { 0.008f, 0.02f, 0.012f }, black);
	AddPart({ 0.0f, 0.055f, -0.44f }, { 0.006f, 0.05f, 0.006f }, black);      //front sight post
	AddPart({ 0.0f, -0.02f, 0.02f }, { 0.074f, 0.01f, 0.16f }, glm::vec3(1.0f), glow); //side light strip
	AddPart({ 0.0f, 0.08f, -0.44f }, { 0.006f, 0.006f, 0.006f }, glm::vec3(1.0f), { 0.4f, 4.0f, 0.4f }); //sight dot

	GameObject& mag = AddPart({ 0.0f, -0.11f, -0.07f }, { 0.05f, 0.16f, 0.08f }, black, glm::vec3(0), { 12.0f, 0.0f, 0.0f });
	magazine = mag.GetComponent<TransformComponent>();
	magazineRest = magazine->GetLocalPosition();
}

void Weapon::BuildShotgun() {
	const glm::vec3 metal(0.13f, 0.13f, 0.15f);
	const glm::vec3 black(0.05f, 0.05f, 0.06f);
	const glm::vec3 wood(0.42f, 0.24f, 0.12f);

	AddPart({ 0.0f, 0.0f, 0.0f }, { 0.075f, 0.1f, 0.32f }, metal);            //receiver
	AddPart({ 0.0f, 0.025f, -0.42f }, { 0.046f, 0.046f, 0.56f }, black);      //barrel
	AddPart({ 0.0f, -0.025f, -0.37f }, { 0.04f, 0.04f, 0.46f }, metal);       //magazine tube
	AddPart({ 0.0f, -0.07f, 0.1f }, { 0.045f, 0.11f, 0.05f }, wood, glm::vec3(0), { -20.0f, 0.0f, 0.0f }); //grip
	AddPart({ 0.0f, -0.035f, 0.28f }, { 0.06f, 0.1f, 0.26f }, wood);          //stock
	AddPart({ 0.0f, 0.055f, -0.68f }, { 0.016f, 0.016f, 0.016f }, glm::vec3(1.0f), { 3.0f, 0.5f, 0.3f }); //bead sight

	GameObject& pump = AddPart({ 0.0f, -0.032f, -0.33f }, { 0.07f, 0.065f, 0.17f }, wood);
	magazine = pump.GetComponent<TransformComponent>();
	magazineRest = magazine->GetLocalPosition();
}

void Weapon::SetVisible(bool visible) {
	for (MeshRenderer* mesh : meshes) mesh->visible = visible;
	if (!visible) {
		flash->visible = false;
		flashLight->intensity = 0.0f;
		flashTimer = 0.0f;
	}
}

void Weapon::SetLowered(bool l) {
	lowered = l;
	//switching away cancels a reload, like most shooters
	if (lowered) reloading = false;
}

void Weapon::Raise() {
	lowered = false;
	lowerAmount = 1.0f;
}

void Weapon::Refill() {
	lowered = false;
	lowerAmount = 0.0f;
	ammo = stats.magazineSize;
	reserve = stats.startReserve;
	reloading = false;
	bloom = 0.0f;
	aimAmount = 0.0f;
}

bool Weapon::AddReserve(int amount) {
	if (reserve >= stats.maxReserve) return false;
	reserve = std::min(reserve + amount, stats.maxReserve);
	return true;
}

void Weapon::EnsureMinimumReserve(int amount) {
	reserve = std::max(reserve, amount);
}

float Weapon::GetReloadProgress() const {
	if (!reloading) return 0.0f;
	float shell = 1.0f - reloadTimer / stats.reloadTime;
	if (stats.reloadPerShell) return (ammo + shell) / static_cast<float>(stats.magazineSize);
	return shell;
}

float Weapon::GetSpread() const {
	return glm::mix(stats.hipSpread, stats.adsSpread, aimAmount) + stats.moveSpread * moveFraction + bloom;
}

void Weapon::StartReload() {
	if (reloading || ammo >= stats.magazineSize || reserve <= 0) return;
	reloading = true;
	reloadTimer = stats.reloadTime;
	//a beat to bring the gun into position before the first shell goes in
	if (stats.reloadPerShell) reloadTimer += 0.2f;
}

glm::vec2 Weapon::Update(float dt, const WeaponInput& input) {
	glm::vec2 recoil(0.0f);
	fireTimer -= dt;
	flashTimer -= dt;
	timeSinceShot += dt;
	moveFraction = input.moveFraction;
	if (timeSinceShot > 0.15f) bloom = MoveTowards(bloom, 0.0f, 6.0f * dt);

	float aimTarget = input.aimHeld && !input.sprinting && !lowered && !reloading ? 1.0f : 0.0f;
	aimAmount = MoveTowards(aimAmount, aimTarget, dt / 0.15f);
	lowerAmount = MoveTowards(lowerAmount, lowered ? 1.0f : 0.0f, dt / 0.18f);

	if (reloading) {
		//shells can be interrupted to shoot, a magazine swap can't
		if (stats.reloadPerShell && input.firePressed && ammo > 0) {
			reloading = false;
		}
		else {
			reloadTimer -= dt;
			if (reloadTimer <= 0.0f) {
				if (stats.reloadPerShell) {
					++ammo;
					--reserve;
					reloadTimer += stats.reloadTime;
					if (ammo >= stats.magazineSize || reserve <= 0) reloading = false;
				}
				else {
					int taken = std::min(stats.magazineSize - ammo, reserve);
					ammo += taken;
					reserve -= taken;
					reloading = false;
				}
			}
		}
	}
	if (input.reloadPressed) StartReload();

	bool ready = !lowered && lowerAmount <= 0.0f && !reloading && fireTimer <= 0.0f;
	bool trigger = stats.automatic ? input.fireHeld : input.firePressed;
	if (ready && trigger) {
		if (ammo > 0) {
			Fire();
			//keep the cadence steady when a frame overshoots the interval
			fireTimer = std::max(fireTimer, -dt) + stats.fireInterval;
			float aimed = glm::mix(1.0f, 0.6f, aimAmount);
			recoil.x = stats.recoilPitch * RandomFloat(0.8f, 1.2f) * aimed;
			recoil.y = stats.recoilYaw * RandomFloat(-1.0f, 1.0f) * aimed;
		}
		else if (input.firePressed) {
			StartReload();
		}
	}
	//reload on its own once the magazine runs dry
	if (ammo == 0 && !reloading && !lowered && timeSinceShot > 0.3f) StartReload();

	UpdateViewmodel(dt, input);
	return recoil;
}

void Weapon::Fire() {
	--ammo;
	timeSinceShot = 0.0f;
	flashTimer = 0.045f;
	kick = 1.0f;

	float spread = GetSpread();
	bloom = std::min(bloom + stats.bloomPerShot, stats.maxBloom);

	glm::vec3 origin = camera->GetPosition();
	glm::quat look = camera->GetQuat();
	glm::vec3 forward = look * glm::vec3(0, 0, -1);
	glm::vec3 right = look * glm::vec3(1, 0, 0);
	glm::vec3 up = look * glm::vec3(0, 1, 0);
	glm::vec3 muzzlePosition = muzzle->GetPosition();

	for (int i = 0; i < stats.pellets; ++i) {
		//random point on a disc, the sqrt keeps it even instead of bunched in the middle
		float radius = std::tan(glm::radians(spread)) * std::sqrt(RandomFloat(0.0f, 1.0f));
		float angle = RandomFloat(0.0f, glm::two_pi<float>());
		glm::vec3 direction = glm::normalize(forward + right * (std::cos(angle) * radius) + up * (std::sin(angle) * radius));

		glm::vec3 end = origin + direction * stats.range;
		RaycastHit hit;
		if (game.scene.Raycast(origin, direction, stats.range, hit, shooter)) {
			end = hit.point;
			game.ResolveBulletHit(hit, direction, stats.damage, stats.headshotMultiplier, stats.impulse);
		}
		if (i < 3) game.effects.Tracer(muzzlePosition, end);
	}
}

void Weapon::UpdateViewmodel(float dt, const WeaponInput& input) {
	bool rifle = kind == WeaponKind::Rifle;

	//walk bob, mostly gone when aiming so the sights stay usable
	bobAmount = Damp(bobAmount, input.grounded ? input.moveFraction : 0.0f, 10.0f, dt);
	bobPhase += dt * (7.0f + 3.0f * input.moveFraction) * (input.sprinting ? 1.3f : 1.0f);
	float bobScale = bobAmount * (1.0f - aimAmount * 0.85f) * (input.sprinting ? 1.8f : 1.0f);
	glm::vec3 bob = glm::vec3(std::cos(bobPhase) * 0.012f, -std::abs(std::sin(bobPhase)) * 0.014f, 0.0f) * bobScale;

	//the gun lags behind the view when turning
	glm::vec2 swayTarget = glm::clamp(glm::vec2(-input.lookDelta.x, input.lookDelta.y) * 0.0008f, -0.03f, 0.03f);
	sway = Damp(sway, swayTarget * (1.0f - aimAmount * 0.7f), 12.0f, dt);

	kick = Damp(kick, 0.0f, 16.0f, dt);
	float reloadPose = reloading ? 1.0f : 0.0f;
	float sprintPose = input.sprinting ? 1.0f : 0.0f;

	glm::vec3 position = glm::mix(stats.hipPosition, stats.adsPosition, aimAmount) + bob + glm::vec3(sway, 0.0f);
	position.z += kick * (rifle ? 0.03f : 0.08f);
	position.y += -lowerAmount * 0.3f - reloadPose * 0.05f - sprintPose * 0.04f;
	root->SetPosition(Damp(root->GetLocalPosition(), position, 30.0f, dt));

	glm::vec3 rotation(
		kick * (rifle ? 2.0f : 9.0f) - lowerAmount * 35.0f - reloadPose * 10.0f - sprintPose * 12.0f,
		sway.x * 150.0f + sprintPose * 25.0f,
		reloadPose * 25.0f + sway.x * 80.0f);
	glm::quat target = glm::quat(glm::radians(rotation));
	root->SetRotation(glm::slerp(root->GetLocalQuat(), target, 1.0f - std::exp(-18.0f * dt)));

	//rifle magazine drops out and back in, the shotgun pump racks back after each shot
	glm::vec3 offset(0.0f);
	if (rifle && reloading) {
		float t = GetReloadProgress();
		float out = t < 0.25f ? t / 0.25f : (t < 0.6f ? 1.0f : 1.0f - (t - 0.6f) / 0.4f);
		offset.y = -0.22f * out;
	}
	if (!rifle && timeSinceShot > 0.12f && timeSinceShot < 0.52f) {
		offset.z = 0.09f * std::sin((timeSinceShot - 0.12f) / 0.4f * glm::pi<float>());
	}
	magazine->SetPosition(magazineRest + offset);

	bool showFlash = flashTimer > 0.0f && !meshes.empty() && meshes.front()->visible;
	flash->visible = showFlash;
	flashLight->intensity = showFlash ? 3.5f : 0.0f;
	if (showFlash) {
		TransformComponent* flashTransform = flash->getOwner()->GetComponent<TransformComponent>();
		float size = RandomFloat(0.035f, 0.06f) * (rifle ? 1.0f : 1.7f);
		flashTransform->SetSize(glm::vec3(size, size, size * 2.5f));
		flashTransform->SetRotation(glm::vec3(0.0f, 0.0f, RandomFloat(0.0f, 90.0f)));
	}
}
