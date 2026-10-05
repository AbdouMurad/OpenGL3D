#include "FPSGame.h"

#include <algorithm>
#include <random>

static constexpr float ArenaHalfSize = 24.0f;
static constexpr float WallHeight = 6.0f;
static const glm::vec3 PlayerSpawn(0.0f, Player::HalfHeight + 0.02f, 14.0f);

static const glm::vec4 White(1.0f, 1.0f, 1.0f, 0.95f);
static const glm::vec4 Grey(0.7f, 0.72f, 0.75f, 0.9f);
static const glm::vec4 Accent(1.0f, 0.62f, 0.15f, 1.0f);
static const glm::vec4 Danger(1.0f, 0.25f, 0.2f, 1.0f);
static const glm::vec4 Good(0.4f, 1.0f, 0.5f, 1.0f);

//------------------------------------------------------------------ setup

void FPSGame::Start(float width, float height) {
	cubeModel = AssetManager::Get().LoadModel("assets/models/cube.gltf");
	sphereModel = AssetManager::Get().LoadModel("assets/models/sphere.gltf");
	crateModel = AssetManager::Get().LoadModel("assets/models/crate/crate.gltf");

	//a dusky sky, the arena lights do the rest
	clearColor = glm::vec3(0.05f, 0.055f, 0.08f);

	effects.Init(scene, cubeModel);
	BuildArena();
	CreatePlayer(width, height);
	player->Respawn(PlayerSpawn, 0.0f);
	SetState(GameState::Title);
}

void FPSGame::BuildArena() {
	const float half = ArenaHalfSize;

	//one collider for the whole floor, checkered tiles on top so movement is easy to read
	GameObject& floor = scene.CreateObject();
	floor.GetComponent<TransformComponent>()->SetPosition({ 0.0f, -0.5f, 0.0f });
	ColliderComponent* floorCollider = floor.AddComponent<ColliderComponent>(SHAPE::Box);
	static_cast<Box*>(floorCollider->shape.get())->halfExtent = glm::vec3(half, 0.5f, half);

	const float tile = 6.0f;
	const int tiles = static_cast<int>(2.0f * half / tile);
	for (int x = 0; x < tiles; ++x) {
		for (int z = 0; z < tiles; ++z) {
			glm::vec3 center(-half + tile * (x + 0.5f), -0.5f, -half + tile * (z + 0.5f));
			glm::vec3 tint = (x + z) % 2 == 0 ? glm::vec3(0.34f, 0.34f, 0.37f) : glm::vec3(0.27f, 0.27f, 0.3f);
			CreatePart(scene, cubeModel, nullptr, center, glm::vec3(tile, 1.0f, tile), tint);
		}
	}

	//outer walls with a glowing strip along the bottom
	const glm::vec3 wallColor(0.45f, 0.42f, 0.4f);
	const glm::vec3 trimGlow(2.2f, 0.9f, 0.15f);
	CreateStaticBox(scene, cubeModel, { 0.0f, WallHeight * 0.5f, -half - 0.5f }, { 2.0f * half + 2.0f, WallHeight, 1.0f }, wallColor);
	CreateStaticBox(scene, cubeModel, { 0.0f, WallHeight * 0.5f, half + 0.5f }, { 2.0f * half + 2.0f, WallHeight, 1.0f }, wallColor);
	CreateStaticBox(scene, cubeModel, { half + 0.5f, WallHeight * 0.5f, 0.0f }, { 1.0f, WallHeight, 2.0f * half }, wallColor);
	CreateStaticBox(scene, cubeModel, { -half - 0.5f, WallHeight * 0.5f, 0.0f }, { 1.0f, WallHeight, 2.0f * half }, wallColor);
	CreatePart(scene, cubeModel, nullptr, { 0.0f, 0.15f, -half + 0.02f }, { 2.0f * half, 0.08f, 0.06f }, glm::vec3(1.0f), trimGlow);
	CreatePart(scene, cubeModel, nullptr, { 0.0f, 0.15f, half - 0.02f }, { 2.0f * half, 0.08f, 0.06f }, glm::vec3(1.0f), trimGlow);
	CreatePart(scene, cubeModel, nullptr, { half - 0.02f, 0.15f, 0.0f }, { 0.06f, 0.08f, 2.0f * half }, glm::vec3(1.0f), trimGlow);
	CreatePart(scene, cubeModel, nullptr, { -half + 0.02f, 0.15f, 0.0f }, { 0.06f, 0.08f, 2.0f * half }, glm::vec3(1.0f), trimGlow);

	//tall pillars to break line of sight
	const glm::vec3 pillarColor(0.5f, 0.48f, 0.46f);
	for (float x : { -9.0f, 9.0f }) {
		for (float z : { -9.0f, 9.0f }) {
			CreateStaticBox(scene, cubeModel, { x, WallHeight * 0.5f, z }, { 2.0f, WallHeight, 2.0f }, pillarColor);
			CreatePart(scene, cubeModel, nullptr, { x, 2.2f, z }, { 2.04f, 0.1f, 2.04f }, glm::vec3(1.0f), trimGlow);
		}
	}

	//waist high walls to shoot over
	const glm::vec3 coverColor(0.38f, 0.4f, 0.36f);
	const glm::vec3 lowWalls[][2] = {
		{ { 0.0f, 0.6f, -7.0f }, { 5.0f, 1.2f, 0.6f } },
		{ { 0.0f, 0.6f, 7.0f }, { 5.0f, 1.2f, 0.6f } },
		{ { -7.0f, 0.6f, 0.0f }, { 0.6f, 1.2f, 5.0f } },
		{ { 7.0f, 0.6f, 0.0f }, { 0.6f, 1.2f, 5.0f } },
		{ { -16.0f, 0.6f, -4.0f }, { 0.6f, 1.2f, 4.0f } },
		{ { 16.0f, 0.6f, 4.0f }, { 0.6f, 1.2f, 4.0f } },
		{ { 4.0f, 0.6f, -16.0f }, { 4.0f, 1.2f, 0.6f } },
		{ { -4.0f, 0.6f, 16.0f }, { 4.0f, 1.2f, 0.6f } },
	};
	for (const auto& wall : lowWalls) CreateStaticBox(scene, cubeModel, wall[0], wall[1], coverColor);

	//crates are real physics objects: they can be pushed and shot around
	const glm::vec3 crates[] = {
		{ -15.0f, 0.5f, -15.0f }, { -13.9f, 0.5f, -15.0f }, { -15.0f, 1.5f, -15.0f }, { -14.5f, 0.5f, -13.8f },
		{ 15.0f, 0.5f, 15.0f }, { 13.9f, 0.5f, 15.0f }, { 15.0f, 1.5f, 15.0f }, { 14.5f, 0.5f, 13.8f },
		{ 15.0f, 0.5f, -13.0f }, { 15.0f, 0.5f, -11.9f },
		{ -15.0f, 0.5f, 13.0f }, { -15.0f, 0.5f, 11.9f },
		{ 3.0f, 0.5f, -3.0f }, { -3.0f, 0.5f, 3.0f },
	};
	for (const glm::vec3& position : crates) {
		GameObject& crate = scene.CreateObject();
		crate.GetComponent<TransformComponent>()->SetPosition(position);
		crate.AddComponent<MeshRenderer>(crateModel);
		RigidBodyComponent* rb = crate.AddComponent<RigidBodyComponent>();
		rb->SetMass(25.0f);
		crate.AddComponent<ColliderComponent>(SHAPE::Box); //default half extent 0.5 matches the model
		props.push_back({ &crate, position, glm::quat(1, 0, 0, 0) });
	}

	//glowing pads where the robots come in
	spawnPoints = {
		{ 0.0f, 0.0f, -20.0f }, { 0.0f, 0.0f, 20.0f }, { 20.0f, 0.0f, 0.0f }, { -20.0f, 0.0f, 0.0f },
		{ -19.0f, 0.0f, -19.0f }, { 19.0f, 0.0f, 19.0f }, { 19.0f, 0.0f, -19.0f }, { -19.0f, 0.0f, 19.0f },
	};
	for (const glm::vec3& point : spawnPoints) {
		CreatePart(scene, cubeModel, nullptr, point + glm::vec3(0.0f, 0.01f, 0.0f), { 2.6f, 0.04f, 2.6f }, glm::vec3(0.1f), { 0.9f, 0.12f, 0.08f });
	}

	//overhead lights, plus a red glow at each wall gate
	for (float x : { -14.0f, 0.0f, 14.0f }) {
		for (float z : { -14.0f, 0.0f, 14.0f }) {
			GameObject& light = scene.CreateObject();
			light.GetComponent<TransformComponent>()->SetPosition({ x, 9.0f, z });
			light.AddComponent<PointLightComponent>(glm::vec3(1.0f, 0.9f, 0.78f), 3.2f, 26.0f);
		}
	}
	for (int i = 0; i < 4; ++i) {
		GameObject& light = scene.CreateObject();
		light.GetComponent<TransformComponent>()->SetPosition(spawnPoints[i] + glm::vec3(0.0f, 1.5f, 0.0f));
		light.AddComponent<PointLightComponent>(glm::vec3(1.0f, 0.25f, 0.15f), 2.0f, 9.0f);
	}
}

void FPSGame::CreatePlayer(float width, float height) {
	GameObject& body = scene.CreateObject();
	playerObject = &body;
	body.GetComponent<TransformComponent>()->SetPosition(PlayerSpawn);

	RigidBodyComponent* rb = body.AddComponent<RigidBodyComponent>();
	rb->SetMass(80.0f);
	rb->SetConstraint(RigidBodyConstraints::FreezeRotation);
	ColliderComponent* collider = body.AddComponent<ColliderComponent>(SHAPE::Box);
	static_cast<Box*>(collider->shape.get())->halfExtent = glm::vec3(Player::Radius, Player::HalfHeight, Player::Radius);
	collider->friction = 0.0f; //movement sets the velocity directly, friction would make walls sticky

	GameObject& camera = scene.CreateObject();
	camera.GetComponent<TransformComponent>()->parent = body.GetComponent<TransformComponent>();
	CameraComponent* cameraComponent = camera.AddComponent<CameraComponent>(static_cast<int>(width), static_cast<int>(height));
	cameraComponent->SetViewport(static_cast<int>(width), static_cast<int>(height));
	cameraComponent->nearPlane = 0.05f; //the gun is close to the eye
	cameraComponent->farPlane = 200.0f;
	//a little light of our own so the gun and nearby walls aren't silhouettes
	camera.AddComponent<PointLightComponent>(glm::vec3(1.0f, 0.95f, 0.85f), 1.0f, 10.0f);
	scene.SetMainCamera(camera);

	player = body.AddComponent<Player>(this, &camera);
}

//------------------------------------------------------------------ game flow

void FPSGame::SetState(GameState next) {
	state = next;
	stateTime = 0.0f;
	paused = next == GameState::Paused;
	Input::SetCursorLocked(next == GameState::Playing || next == GameState::Dead);
	player->SetControlsEnabled(next == GameState::Playing);
}

void FPSGame::StartGame() {
	ClearWorld();
	player->Respawn(PlayerSpawn, 0.0f);

	wave = 0;
	waveActive = false;
	intermissionTimer = 4.0f;
	spawnQueue.clear();
	score = 0;
	kills = 0;
	headshots = 0;
	feed.clear();
	hitMarkerTimer = 0.0f;

	SetState(GameState::Playing);
	ShowBanner("GET READY", "WAVE 1 INCOMING", 3.5f);
}

void FPSGame::ClearWorld() {
	for (Enemy* enemy : enemies) scene.Destroy(*enemy->getOwner());
	enemies.clear();
	for (Projectile& projectile : projectiles) scene.Destroy(*projectile.object);
	projectiles.clear();
	for (Pickup& pickup : pickups) scene.Destroy(*pickup.object);
	pickups.clear();
	effects.Clear();

	for (Prop& prop : props) {
		TransformComponent* transform = prop.object->GetComponent<TransformComponent>();
		transform->SetPosition(prop.position);
		transform->SetRotation(prop.rotation);
		RigidBodyComponent* rb = prop.object->GetComponent<RigidBodyComponent>();
		rb->SetVelocity(glm::vec3(0.0f));
		rb->SetAngularVelocity(glm::vec3(0.0f));
	}
}

void FPSGame::Update(float dt) {
	stateTime += dt;

	switch (state) {
	case GameState::Title:
		if (Input::GetMouseButtonDown(0) || Input::GetKeyDown(GLFW_KEY_ENTER)) StartGame();
		else if (Input::GetKeyDown(GLFW_KEY_ESCAPE)) quitRequested = true;
		break;
	case GameState::Paused:
		if (Input::GetKeyDown(GLFW_KEY_ESCAPE) || Input::GetMouseButtonDown(0)) SetState(GameState::Playing);
		else if (Input::GetKeyDown(GLFW_KEY_Q)) quitRequested = true;
		//everything is frozen, including the effects
		return;
	case GameState::Dead:
		if (Input::GetKeyDown(GLFW_KEY_R) && stateTime > 1.0f) StartGame();
		else if (Input::GetKeyDown(GLFW_KEY_Q)) quitRequested = true;
		break;
	case GameState::Playing:
		if (Input::GetKeyDown(GLFW_KEY_ESCAPE)) {
			SetState(GameState::Paused);
			return;
		}
		if (player->IsDead()) {
			bestScore = std::max(bestScore, score);
			SetState(GameState::Dead);
			break;
		}
		UpdateWaves(dt);
		break;
	}

	UpdateProjectiles(dt);
	UpdatePickups(dt);
	effects.Update(dt);
	DespawnEnemies();
	UpdateHUDTimers(dt);
}

//------------------------------------------------------------------ waves

void FPSGame::UpdateWaves(float dt) {
	if (!waveActive) {
		intermissionTimer -= dt;
		if (intermissionTimer <= 0.0f) StartWave(wave + 1);
		return;
	}

	//trickle robots in from the queue, with a cap on how many are up at once
	spawnTimer -= dt;
	int alive = AliveEnemies();
	int maxAlive = std::min(6 + wave, 12);
	if (!spawnQueue.empty() && spawnTimer <= 0.0f && alive < maxAlive) {
		SpawnEnemy(spawnQueue.back());
		spawnQueue.pop_back();
		spawnTimer = 0.8f;
	}

	if (spawnQueue.empty() && alive == 0) {
		waveActive = false;
		intermissionTimer = 6.0f;
		int bonus = wave * 250;
		score += bonus;
		player->Heal(25.0f);
		ShowBanner("WAVE " + std::to_string(wave) + " CLEARED", "+" + std::to_string(bonus) + " BONUS", 3.5f);
	}
}

void FPSGame::StartWave(int number) {
	wave = number;
	waveActive = true;
	spawnTimer = 0.3f;

	int grunts = 5;
	int gunners = 0;
	int brutes = 0;
	if (wave == 2) {
		grunts = 6;
		gunners = 2;
	}
	else if (wave >= 3) {
		grunts = std::min(4 + wave, 14);
		gunners = std::min(wave, 8);
		brutes = std::min(wave - 2, 5);
	}
	spawnQueue.clear();
	spawnQueue.insert(spawnQueue.end(), grunts, EnemyType::Grunt);
	spawnQueue.insert(spawnQueue.end(), gunners, EnemyType::Gunner);
	spawnQueue.insert(spawnQueue.end(), brutes, EnemyType::Brute);
	static std::mt19937 shuffleRng(std::random_device{}());
	std::shuffle(spawnQueue.begin(), spawnQueue.end(), shuffleRng);

	//never start a wave with nothing to shoot with
	player->EnsureMinimumAmmo();
	std::string subtitle = brutes > 0 ? "BRUTES INCOMING" : (gunners > 0 ? "WATCH FOR PLASMA FIRE" : "SURVIVE");
	ShowBanner("WAVE " + std::to_string(wave), subtitle, 2.5f);
}

void FPSGame::SpawnEnemy(EnemyType type) {
	//only use spawn points well away from the player
	glm::vec3 playerPosition = player->GetPosition();
	std::vector<glm::vec3> candidates;
	for (const glm::vec3& point : spawnPoints) {
		glm::vec2 offset(point.x - playerPosition.x, point.z - playerPosition.z);
		if (glm::length(offset) > 14.0f) candidates.push_back(point);
	}
	if (candidates.empty()) candidates = spawnPoints;
	glm::vec3 point = candidates[RandomInt(0, static_cast<int>(candidates.size()) - 1)];
	point += glm::vec3(RandomFloat(-1.0f, 1.0f), 0.0f, RandomFloat(-1.0f, 1.0f));

	float healthScale = 1.0f + 0.1f * (wave - 1);
	float speedScale = std::min(1.0f + 0.03f * (wave - 1), 1.3f);
	Enemy& enemy = Enemy::Spawn(*this, type, point, healthScale, speedScale);
	enemies.push_back(&enemy);

	glm::vec3 glow = enemy.Stats().visorColor;
	effects.Burst(point + glm::vec3(0.0f, 1.0f, 0.0f), glow / std::max(glow.r, std::max(glow.g, glow.b)), 16, 4.0f, 0.08f);
}

int FPSGame::AliveEnemies() const {
	return static_cast<int>(std::count_if(enemies.begin(), enemies.end(), [](const Enemy* e) { return !e->IsDead(); }));
}

void FPSGame::DespawnEnemies() {
	for (size_t i = 0; i < enemies.size();) {
		if (enemies[i]->ShouldDespawn()) {
			scene.Destroy(*enemies[i]->getOwner());
			enemies.erase(enemies.begin() + i);
		}
		else {
			++i;
		}
	}
}

//------------------------------------------------------------------ combat

void FPSGame::ResolveBulletHit(const RaycastHit& hit, const glm::vec3& direction, float damage, float headshotMultiplier, float impulse) {
	GameObject* object = hit.collider->getOwner();

	if (Enemy* enemy = object->GetComponent<Enemy>()) {
		effects.Blood(hit.point, glm::normalize(hit.normal - direction), 3);
		if (enemy->IsDead()) {
			enemy->Push(hit.point, direction * impulse * 10.0f);
			return;
		}
		bool headshot = hit.collider->GetID() == "head";
		bool killed = enemy->TakeDamage(damage * (headshot ? headshotMultiplier : 1.0f), hit.point, direction);

		//a kill marker isn't downgraded by the shotgun's other pellets in the same frame
		if (hitMarkerTimer <= 0.1f) hitMarkerKill = false;
		hitMarkerTimer = 0.2f;
		hitMarkerKill = hitMarkerKill || killed;
		hitMarkerHead = headshot;
		if (killed) OnEnemyKilled(*enemy, headshot);
		return;
	}

	effects.Sparks(hit.point, hit.normal, 5);
	RigidBodyComponent* rb = object->GetComponent<RigidBodyComponent>();
	if (rb && !rb->IsStatic()) {
		rb->AddImpulse(direction * impulse);
		rb->AddAngularImpulse(glm::cross(hit.point - rb->GetCenterMass(), direction * impulse));
	}
	else if (object != playerObject) {
		//holes only on things that never move, they'd be left floating otherwise
		effects.BulletHole(hit.point, hit.normal);
	}
}

void FPSGame::OnEnemyKilled(Enemy& enemy, bool headshot) {
	++kills;
	if (headshot) ++headshots;
	int points = enemy.Stats().score + (headshot ? 50 : 0);
	score += points;
	AddFeed((headshot ? "HEADSHOT  +" : "KILL  +") + std::to_string(points), headshot ? Accent : White);

	glm::vec3 position = enemy.GetPosition();
	effects.Burst(position, glm::vec3(1.0f, 0.55f, 0.15f), 12, 5.0f, 0.07f);

	//loot: health is likelier when it's needed, brutes always drop something
	float healthChance = player->GetHealth() < 50.0f ? 0.35f : 0.15f;
	float roll = RandomFloat(0.0f, 1.0f);
	if (enemy.GetType() == EnemyType::Brute) roll *= healthChance + 0.3f;
	if (roll < healthChance) SpawnPickup(PickupType::Health, position);
	else if (roll < healthChance + 0.3f) SpawnPickup(PickupType::Ammo, position);
}

void FPSGame::SpawnProjectile(const glm::vec3& position, const glm::vec3& velocity, float damage, GameObject* owner) {
	GameObject& orb = CreatePart(scene, sphereModel, nullptr, position, glm::vec3(0.26f), glm::vec3(1.0f), glm::vec3(0.6f, 2.6f, 4.0f));
	orb.AddComponent<PointLightComponent>(glm::vec3(0.3f, 0.7f, 1.0f), 2.5f, 6.0f);
	projectiles.push_back({ &orb, velocity, damage, 4.0f, owner });
	effects.Burst(position, glm::vec3(0.3f, 0.8f, 1.0f), 4, 1.5f, 0.05f);
}

bool FPSGame::SegmentHitsPlayer(const glm::vec3& from, const glm::vec3& to, float radius) const {
	//the body as a capsule, friendlier than the box for something round and fast
	glm::vec3 center = player->GetPosition();
	glm::vec3 bodyOffset(0.0f, Player::HalfHeight - Player::Radius, 0.0f);
	glm::vec3 onSegment, onBody;
	Collision::ClosestPointsBetweenEdges(from, to, center - bodyOffset, center + bodyOffset, onSegment, onBody);
	float reach = Player::Radius + radius;
	return glm::length2(onSegment - onBody) < reach * reach;
}

void FPSGame::UpdateProjectiles(float dt) {
	for (size_t i = 0; i < projectiles.size();) {
		Projectile& projectile = projectiles[i];
		TransformComponent* transform = projectile.object->GetComponent<TransformComponent>();
		glm::vec3 from = transform->GetLocalPosition();
		glm::vec3 step = projectile.velocity * dt;
		glm::vec3 to = from + step;
		float length = glm::length(step);
		projectile.life -= dt;

		bool explode = projectile.life <= 0.0f;
		glm::vec3 impact = to;
		if (!player->IsDead() && SegmentHitsPlayer(from, to, 0.13f)) {
			player->TakeDamage(projectile.damage, from);
			explode = true;
		}
		else if (length > 1e-5f) {
			RaycastHit hit;
			if (scene.Raycast(from, step / length, length, hit, projectile.owner)) {
				explode = true;
				impact = hit.point;
			}
		}

		if (explode) {
			effects.Burst(impact, glm::vec3(0.3f, 0.8f, 1.0f), 10, 3.5f, 0.06f);
			scene.Destroy(*projectile.object);
			projectiles[i] = projectiles.back();
			projectiles.pop_back();
			continue;
		}
		transform->SetPosition(to);
		//flicker
		transform->SetSize(glm::vec3(0.26f * RandomFloat(0.85f, 1.15f)));
		++i;
	}
}

void FPSGame::SpawnPickup(PickupType type, const glm::vec3& position) {
	glm::vec3 base(position.x, 0.55f, position.z);
	GameObject& root = scene.CreateObject();
	TransformComponent* transform = root.GetComponent<TransformComponent>();
	transform->SetPosition(base);

	if (type == PickupType::Health) {
		const glm::vec3 tint(0.3f, 1.0f, 0.4f);
		const glm::vec3 glow(0.1f, 1.6f, 0.25f);
		CreatePart(scene, cubeModel, transform, glm::vec3(0.0f), { 0.42f, 0.13f, 0.13f }, tint, glow);
		CreatePart(scene, cubeModel, transform, glm::vec3(0.0f), { 0.13f, 0.42f, 0.13f }, tint, glow);
	}
	else {
		CreatePart(scene, cubeModel, transform, glm::vec3(0.0f), { 0.38f, 0.24f, 0.26f }, { 0.35f, 0.33f, 0.15f });
		CreatePart(scene, cubeModel, transform, { 0.0f, 0.125f, 0.0f }, { 0.3f, 0.02f, 0.18f }, glm::vec3(1.0f), { 3.0f, 2.0f, 0.3f });
	}
	pickups.push_back({ &root, type, base, 0.0f });
}

void FPSGame::UpdatePickups(float dt) {
	const float lifetime = 25.0f;
	glm::vec3 playerPosition = player->GetPosition();
	for (size_t i = 0; i < pickups.size();) {
		Pickup& pickup = pickups[i];
		pickup.age += dt;
		TransformComponent* transform = pickup.object->GetComponent<TransformComponent>();
		transform->SetPosition(pickup.basePosition + glm::vec3(0.0f, std::sin(pickup.age * 3.0f) * 0.08f, 0.0f));
		transform->SetRotation(glm::angleAxis(pickup.age * 1.8f, glm::vec3(0.0f, 1.0f, 0.0f)));
		//shrink away near the end so it's clear it's about to go
		transform->SetSize(glm::vec3(glm::clamp((lifetime - pickup.age) / 3.0f, 0.01f, 1.0f)));

		bool collected = false;
		glm::vec3 offset = playerPosition - pickup.basePosition;
		if (state == GameState::Playing && glm::length(glm::vec2(offset.x, offset.z)) < 1.2f && std::abs(offset.y) < 1.6f) {
			if (pickup.type == PickupType::Health && player->Heal(30.0f)) {
				AddFeed("+30 HEALTH", Good);
				collected = true;
			}
			else if (pickup.type == PickupType::Ammo && player->AddAmmo()) {
				AddFeed("+AMMO", Accent);
				collected = true;
			}
		}

		if (collected || pickup.age > lifetime) {
			if (collected) effects.Burst(pickup.basePosition, pickup.type == PickupType::Health ? glm::vec3(0.3f, 1.0f, 0.4f) : glm::vec3(1.0f, 0.8f, 0.2f), 10, 3.0f, 0.05f);
			scene.Destroy(*pickup.object);
			pickups[i] = pickups.back();
			pickups.pop_back();
			continue;
		}
		++i;
	}
}

//------------------------------------------------------------------ HUD

void FPSGame::AddFeed(const std::string& text, const glm::vec4& color) {
	feed.push_front({ text, color, 2.5f });
	if (feed.size() > 5) feed.pop_back();
}

void FPSGame::ShowBanner(const std::string& title, const std::string& subtitle, float duration) {
	bannerTitle = title;
	bannerSubtitle = subtitle;
	bannerTimer = duration;
	bannerDuration = duration;
}

void FPSGame::UpdateHUDTimers(float dt) {
	hitMarkerTimer -= dt;
	bannerTimer -= dt;
	for (FeedEntry& entry : feed) entry.timer -= dt;
	while (!feed.empty() && feed.back().timer <= 0.0f) feed.pop_back();
}

void FPSGame::RenderUI(UIRenderer& ui) {
	float s = ui.Height() / 1080.0f;

	if (state == GameState::Title) {
		DrawTitle(ui, s);
		return;
	}
	DrawDamage(ui, s);
	if (state != GameState::Dead) DrawHUD(ui, s);
	if (state == GameState::Paused) DrawPause(ui, s);
	if (state == GameState::Dead) DrawDeath(ui, s);
}

void FPSGame::DrawCrosshair(UIRenderer& ui, float s) {
	float cx = ui.Width() * 0.5f;
	float cy = ui.Height() * 0.5f;
	Weapon& weapon = player->CurrentWeapon();

	//the gap shows the real spread: where the cone edge lands on screen
	float pixelsPerUnit = (ui.Height() * 0.5f) / std::tan(glm::radians(player->GetFov() * 0.5f));
	float gap = std::tan(glm::radians(weapon.GetSpread())) * pixelsPerUnit + 4.0f * s;
	float alpha = 0.9f * (1.0f - weapon.GetAimAmount());
	if (alpha > 0.02f) {
		float length = 10.0f * s;
		float thickness = 2.0f * s;
		glm::vec4 outline(0.0f, 0.0f, 0.0f, alpha * 0.5f);
		glm::vec4 color(1.0f, 1.0f, 1.0f, alpha);
		const glm::vec4 bars[4] = {
			{ cx - thickness * 0.5f, cy - gap - length, thickness, length },
			{ cx - thickness * 0.5f, cy + gap, thickness, length },
			{ cx - gap - length, cy - thickness * 0.5f, length, thickness },
			{ cx + gap, cy - thickness * 0.5f, length, thickness },
		};
		for (const glm::vec4& bar : bars) ui.DrawRect(bar.x - s, bar.y - s, bar.z + 2.0f * s, bar.w + 2.0f * s, outline);
		for (const glm::vec4& bar : bars) ui.DrawRect(bar.x, bar.y, bar.z, bar.w, color);
		ui.DrawRect(cx - s, cy - s, 2.0f * s, 2.0f * s, color);
	}

	if (hitMarkerTimer > 0.0f) {
		float a = std::min(hitMarkerTimer / 0.2f, 1.0f);
		glm::vec4 color = hitMarkerKill ? Danger : (hitMarkerHead ? Accent : White);
		color.a = a;
		float outer = (hitMarkerKill ? 22.0f : 15.0f) * s;
		float dot = 3.0f * s;
		//an X made of small squares along the diagonals
		for (float d = 7.0f * s; d < outer; d += 1.5f * s) {
			ui.DrawRect(cx + d - dot * 0.5f, cy + d - dot * 0.5f, dot, dot, color);
			ui.DrawRect(cx - d - dot * 0.5f, cy + d - dot * 0.5f, dot, dot, color);
			ui.DrawRect(cx + d - dot * 0.5f, cy - d - dot * 0.5f, dot, dot, color);
			ui.DrawRect(cx - d - dot * 0.5f, cy - d - dot * 0.5f, dot, dot, color);
		}
	}
}

void FPSGame::DrawHUD(UIRenderer& ui, float s) {
	float W = static_cast<float>(ui.Width());
	float H = static_cast<float>(ui.Height());
	Weapon& weapon = player->CurrentWeapon();

	DrawCrosshair(ui, s);

	//reload / ammo prompts under the crosshair
	if (weapon.IsReloading()) {
		ui.DrawTextCentered("RELOADING", W * 0.5f, H * 0.5f + 90.0f * s, 2.2f * s, White);
		float barWidth = 120.0f * s;
		ui.DrawRect(W * 0.5f - barWidth * 0.5f, H * 0.5f + 114.0f * s, barWidth, 4.0f * s, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
		ui.DrawRect(W * 0.5f - barWidth * 0.5f, H * 0.5f + 114.0f * s, barWidth * weapon.GetReloadProgress(), 4.0f * s, Accent);
	}
	else if (weapon.GetAmmo() == 0 && weapon.GetReserve() == 0) {
		ui.DrawTextCentered("NO AMMO - SWITCH WEAPON", W * 0.5f, H * 0.5f + 90.0f * s, 2.2f * s, Danger);
	}
	else if (weapon.GetAmmo() <= weapon.Stats().magazineSize / 4) {
		ui.DrawTextCentered("R  RELOAD", W * 0.5f, H * 0.5f + 90.0f * s, 2.0f * s, glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
	}

	//health, bottom left
	float health = player->GetHealth();
	float fraction = health / player->GetMaxHealth();
	float x = 48.0f * s;
	float y = H - 80.0f * s;
	float barWidth = 320.0f * s;
	glm::vec4 healthColor = glm::mix(Danger, Good, fraction);
	if (fraction < 0.3f) healthColor.a = 0.6f + 0.4f * std::sin(stateTime * 10.0f);
	ui.DrawText("HEALTH", x, y - 28.0f * s, 2.2f * s, Grey);
	ui.DrawRect(x, y, barWidth, 24.0f * s, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
	ui.DrawRect(x + 3.0f * s, y + 3.0f * s, (barWidth - 6.0f * s) * fraction, 18.0f * s, healthColor);
	ui.DrawText(std::to_string(static_cast<int>(std::ceil(health))), x + barWidth + 16.0f * s, y - 2.0f * s, 3.8f * s, White);

	//ammo, bottom right
	float right = W - 48.0f * s;
	std::string reserveText = "/ " + std::to_string(weapon.GetReserve());
	std::string magazineText = std::to_string(weapon.GetAmmo());
	float reserveWidth = ui.TextWidth(reserveText, 3.0f * s);
	float magazineWidth = ui.TextWidth(magazineText, 6.5f * s);
	glm::vec4 ammoColor = weapon.GetAmmo() == 0 ? Danger : (weapon.GetAmmo() <= weapon.Stats().magazineSize / 4 ? Accent : White);
	ui.DrawText(reserveText, right - reserveWidth, H - 66.0f * s, 3.0f * s, Grey);
	ui.DrawText(magazineText, right - reserveWidth - 14.0f * s - magazineWidth, H - 96.0f * s, 6.5f * s, ammoColor);
	std::string name = weapon.Stats().name;
	ui.DrawText(name, right - ui.TextWidth(name, 2.2f * s), H - 130.0f * s, 2.2f * s, Accent);

	//wave and enemy count, top center
	int remaining = AliveEnemies() + static_cast<int>(spawnQueue.size());
	if (wave > 0) ui.DrawTextCentered("WAVE " + std::to_string(wave), W * 0.5f, 28.0f * s, 3.4f * s, White);
	if (waveActive) {
		ui.DrawTextCentered("ENEMIES LEFT  " + std::to_string(remaining), W * 0.5f, 66.0f * s, 2.0f * s, Grey);
	}
	else if (state == GameState::Playing) {
		int seconds = static_cast<int>(std::ceil(std::max(intermissionTimer, 0.0f)));
		ui.DrawTextCentered("NEXT WAVE IN  " + std::to_string(seconds), W * 0.5f, 66.0f * s, 2.0f * s, Accent);
	}

	//score and kill feed, top right
	std::string scoreText = std::to_string(score);
	ui.DrawText("SCORE", right - ui.TextWidth("SCORE", 2.0f * s), 24.0f * s, 2.0f * s, Grey);
	ui.DrawText(scoreText, right - ui.TextWidth(scoreText, 4.2f * s), 48.0f * s, 4.2f * s, White);
	float feedY = 104.0f * s;
	for (const FeedEntry& entry : feed) {
		glm::vec4 color = entry.color;
		color.a *= std::min(entry.timer / 0.5f, 1.0f);
		ui.DrawText(entry.text, right - ui.TextWidth(entry.text, 2.2f * s), feedY, 2.2f * s, color);
		feedY += 28.0f * s;
	}

	//big announcements
	if (bannerTimer > 0.0f) {
		float age = bannerDuration - bannerTimer;
		float alpha = std::min(std::min(age / 0.25f, bannerTimer / 0.5f), 1.0f);
		glm::vec4 titleColor = Accent;
		titleColor.a = alpha;
		glm::vec4 subtitleColor = White;
		subtitleColor.a = alpha * 0.9f;
		ui.DrawTextCentered(bannerTitle, W * 0.5f, H * 0.26f, 8.0f * s, titleColor);
		ui.DrawTextCentered(bannerSubtitle, W * 0.5f, H * 0.26f + 76.0f * s, 3.0f * s, subtitleColor);
	}

	//frame rate, top left
	ui.DrawText("FPS " + std::to_string(static_cast<int>(Time::FPS())), 16.0f * s, 16.0f * s, 1.6f * s, glm::vec4(1.0f, 1.0f, 1.0f, 0.5f), false);
}

void FPSGame::DrawDamage(UIRenderer& ui, float s) {
	float W = static_cast<float>(ui.Width());
	float H = static_cast<float>(ui.Height());

	//red edges when hit, pulsing while health is low
	float intensity = player->damageFlash * 0.55f;
	if (!player->IsDead() && player->GetHealth() < 35.0f) {
		intensity += (0.35f - player->GetHealth() / 100.0f) * (0.6f + 0.4f * std::sin(stateTime * 5.0f));
	}
	intensity = std::min(intensity, 0.8f);
	if (intensity > 0.01f) {
		const int bands = 8;
		float band = 16.0f * s;
		for (int i = 0; i < bands; ++i) {
			float t = 1.0f - static_cast<float>(i) / bands;
			glm::vec4 color(0.8f, 0.0f, 0.0f, intensity * t * t);
			ui.DrawRect(0.0f, i * band, W, band, color);
			ui.DrawRect(0.0f, H - (i + 1) * band, W, band, color);
			ui.DrawRect(i * band, 0.0f, band, H, color);
			ui.DrawRect(W - (i + 1) * band, 0.0f, band, H, color);
		}
	}

	//which way the hit came from, as an arc around the crosshair
	float cx = W * 0.5f;
	float cy = H * 0.5f;
	for (const Player::DamageIndicator& indicator : player->damageIndicators) {
		glm::vec4 color(1.0f, 0.15f, 0.1f, std::min(indicator.timer, 1.0f) * 0.85f);
		for (int row = 0; row < 2; ++row) {
			float radius = (130.0f - row * 9.0f) * s;
			int half = 4 - row;
			for (int j = -half; j <= half; ++j) {
				float angle = indicator.angle + j * 0.055f;
				float px = cx + std::sin(angle) * radius;
				float py = cy - std::cos(angle) * radius;
				ui.DrawRect(px - 4.0f * s, py - 4.0f * s, 8.0f * s, 8.0f * s, color);
			}
		}
	}
}

void FPSGame::DrawTitle(UIRenderer& ui, float s) {
	float W = static_cast<float>(ui.Width());
	float H = static_cast<float>(ui.Height());
	ui.DrawRect(0.0f, 0.0f, W, H, glm::vec4(0.02f, 0.02f, 0.04f, 0.6f));

	ui.DrawTextCentered("IRON ARENA", W * 0.5f, H * 0.18f, 14.0f * s, Accent);
	ui.DrawTextCentered("SURVIVE THE ROBOT WAVES", W * 0.5f, H * 0.18f + 120.0f * s, 3.0f * s, White);

	const char* controls[] = {
		"WASD  MOVE        SHIFT  SPRINT        SPACE  JUMP",
		"LEFT MOUSE  FIRE        RIGHT MOUSE  AIM DOWN SIGHTS",
		"R  RELOAD        1 / 2 / Q / WHEEL  SWITCH WEAPON",
		"ESC  PAUSE        Z  SHOW COLLIDERS",
	};
	float y = H * 0.45f;
	for (const char* line : controls) {
		ui.DrawTextCentered(line, W * 0.5f, y, 2.4f * s, Grey);
		y += 38.0f * s;
	}
	ui.DrawTextCentered("HEADSHOTS DEAL EXTRA DAMAGE.  GLOWING ROBOTS ARE ABOUT TO ATTACK.", W * 0.5f, y + 20.0f * s, 2.2f * s, glm::vec4(1.0f, 1.0f, 1.0f, 0.6f));

	glm::vec4 prompt = White;
	prompt.a = 0.6f + 0.4f * std::sin(stateTime * 4.0f);
	ui.DrawTextCentered("CLICK TO START", W * 0.5f, H * 0.78f, 5.0f * s, prompt);
	if (bestScore > 0) ui.DrawTextCentered("BEST  " + std::to_string(bestScore), W * 0.5f, H * 0.78f + 60.0f * s, 2.4f * s, Accent);
}

void FPSGame::DrawPause(UIRenderer& ui, float s) {
	float W = static_cast<float>(ui.Width());
	float H = static_cast<float>(ui.Height());
	ui.DrawRect(0.0f, 0.0f, W, H, glm::vec4(0.0f, 0.0f, 0.0f, 0.6f));
	ui.DrawTextCentered("PAUSED", W * 0.5f, H * 0.32f, 11.0f * s, White);
	ui.DrawTextCentered("ESC / CLICK  RESUME", W * 0.5f, H * 0.32f + 130.0f * s, 3.0f * s, Grey);
	ui.DrawTextCentered("Q  QUIT", W * 0.5f, H * 0.32f + 175.0f * s, 3.0f * s, Grey);
}

void FPSGame::DrawDeath(UIRenderer& ui, float s) {
	float W = static_cast<float>(ui.Width());
	float H = static_cast<float>(ui.Height());
	float fade = std::min(stateTime / 1.2f, 1.0f);
	ui.DrawRect(0.0f, 0.0f, W, H, glm::vec4(0.2f, 0.0f, 0.0f, 0.5f * fade));

	glm::vec4 title = Danger;
	title.a = fade;
	ui.DrawTextCentered("YOU DIED", W * 0.5f, H * 0.28f, 14.0f * s, title);

	glm::vec4 text = White;
	text.a = fade;
	std::string stats = "WAVE " + std::to_string(wave) + "     KILLS " + std::to_string(kills) +
		"     HEADSHOTS " + std::to_string(headshots) + "     SCORE " + std::to_string(score);
	ui.DrawTextCentered(stats, W * 0.5f, H * 0.28f + 130.0f * s, 3.0f * s, text);
	if (score > 0 && score >= bestScore) {
		glm::vec4 best = Accent;
		best.a = fade;
		ui.DrawTextCentered("NEW BEST", W * 0.5f, H * 0.28f + 175.0f * s, 3.0f * s, best);
	}
	if (stateTime > 1.0f) {
		glm::vec4 prompt = White;
		prompt.a = 0.6f + 0.4f * std::sin(stateTime * 4.0f);
		ui.DrawTextCentered("R  TRY AGAIN        Q  QUIT", W * 0.5f, H * 0.7f, 4.0f * s, prompt);
	}
}
