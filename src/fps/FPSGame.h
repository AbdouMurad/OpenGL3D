#pragma once

#include "Common.h"
#include "Effects.h"
#include "Player.h"
#include "Enemy.h"

#include <deque>
#include <string>

enum class GameState { Title, Playing, Paused, Dead };

// Wave survival arena shooter: robots come in waves, each bigger than the last, until they get you.
class FPSGame : public Game {
public:
	void Start(float width, float height) override;
	void Update(float dt) override;
	void RenderUI(UIRenderer& ui) override;

	//shared with the player, weapons and enemies
	Player& GetPlayer() { return *player; }
	GameObject& GetPlayerObject() { return *playerObject; }
	const std::vector<Enemy*>& GetEnemies() const { return enemies; }

	//damage, effects and scoring for a bullet that hit something
	void ResolveBulletHit(const RaycastHit& hit, const glm::vec3& direction, float damage, float headshotMultiplier, float impulse);
	void SpawnProjectile(const glm::vec3& position, const glm::vec3& velocity, float damage, GameObject* owner);

	Effects effects;
	ModelHandle cubeModel = 0;
	ModelHandle sphereModel = 0;
	ModelHandle crateModel = 0;

private:
	enum class PickupType { Health, Ammo };
	struct Pickup {
		GameObject* object;
		PickupType type;
		glm::vec3 basePosition;
		float age;
	};
	struct Projectile {
		GameObject* object;
		glm::vec3 velocity;
		float damage;
		float life;
		GameObject* owner; //only compared against, it may already be gone
	};
	struct Prop {
		GameObject* object;
		glm::vec3 position;
		glm::quat rotation;
	};
	struct FeedEntry {
		std::string text;
		glm::vec4 color;
		float timer;
	};

	void BuildArena();
	void CreatePlayer(float width, float height);
	void SetState(GameState next);
	void StartGame();
	void ClearWorld();

	void UpdateWaves(float dt);
	void StartWave(int number);
	void SpawnEnemy(EnemyType type);
	int AliveEnemies() const;
	void OnEnemyKilled(Enemy& enemy, bool headshot);
	void DespawnEnemies();

	void UpdateProjectiles(float dt);
	bool SegmentHitsPlayer(const glm::vec3& from, const glm::vec3& to, float radius) const;
	void SpawnPickup(PickupType type, const glm::vec3& position);
	void UpdatePickups(float dt);

	void AddFeed(const std::string& text, const glm::vec4& color);
	void ShowBanner(const std::string& title, const std::string& subtitle, float duration);
	void UpdateHUDTimers(float dt);

	void DrawHUD(UIRenderer& ui, float s);
	void DrawCrosshair(UIRenderer& ui, float s);
	void DrawDamage(UIRenderer& ui, float s);
	void DrawTitle(UIRenderer& ui, float s);
	void DrawPause(UIRenderer& ui, float s);
	void DrawDeath(UIRenderer& ui, float s);

	GameState state = GameState::Title;
	float stateTime = 0.0f;

	Player* player = nullptr;
	GameObject* playerObject = nullptr;

	std::vector<Enemy*> enemies;
	std::vector<Projectile> projectiles;
	std::vector<Pickup> pickups;
	std::vector<Prop> props;
	std::vector<glm::vec3> spawnPoints;

	int wave = 0;
	bool waveActive = false;
	float intermissionTimer = 0.0f;
	std::vector<EnemyType> spawnQueue;
	float spawnTimer = 0.0f;

	int score = 0;
	int bestScore = 0;
	int kills = 0;
	int headshots = 0;

	float hitMarkerTimer = 0.0f;
	bool hitMarkerKill = false;
	bool hitMarkerHead = false;
	std::deque<FeedEntry> feed;
	std::string bannerTitle;
	std::string bannerSubtitle;
	float bannerTimer = 0.0f;
	float bannerDuration = 1.0f;
};
