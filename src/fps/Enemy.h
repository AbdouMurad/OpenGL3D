#pragma once

#include "Common.h"

class FPSGame;

enum class EnemyType { Grunt, Gunner, Brute };

struct EnemyStats {
	float health;
	float speed;
	float damage;
	float attackRange;   //melee reach, or the distance gunners like to keep
	float attackCooldown;
	float windup;        //telegraph before the attack lands, the player's window to react
	float scale;
	float mass;
	int score;
	glm::vec3 color;
	glm::vec3 visorColor;
};

// A robot that hunts the player. The root object carries the physics body, which never rotates while
// alive; the visible parts hang off a child that turns to face the target.
class Enemy : public Component {
public:
	Enemy(FPSGame* game, EnemyType type, float healthScale, float speedScale);
	static Enemy& Spawn(FPSGame& game, EnemyType type, const glm::vec3& feetPosition, float healthScale, float speedScale);

	void Start() override;
	void Update(float dt) override;

	//returns true when this hit killed it
	bool TakeDamage(float amount, const glm::vec3& point, const glm::vec3& direction);
	//shove at a point, spin scales how much of it turns into tumbling (only once dead, alive ones stay upright)
	void Push(const glm::vec3& point, const glm::vec3& impulse, float spin = 0.3f);

	EnemyType GetType() const { return type; }
	const EnemyStats& Stats() const { return stats; }
	bool IsDead() const { return state == State::Dead; }
	bool ShouldDespawn() const { return IsDead() && stateTimer > 4.0f; }
	glm::vec3 GetPosition() const;

private:
	enum class State { Chase, Windup, Recover, Dead };

	void Build();
	TransformComponent* AddPart(TransformComponent* parent, const glm::vec3& position, const glm::vec3& size,
		const glm::vec3& tint, const glm::vec3& emissive = glm::vec3(0));
	TransformComponent* AddPivot(TransformComponent* parent, const glm::vec3& position);

	glm::vec3 GunnerMovement(const glm::vec3& toPlayer, float distance, float dt);
	glm::vec3 Steer(const glm::vec3& desired);
	bool IsStaticObstacle(ColliderComponent* collider) const;
	bool HasLineOfSight();
	void Attack();
	void Die(const glm::vec3& point, const glm::vec3& direction);
	void Animate(float dt);

	FPSGame* game = nullptr;
	EnemyType type;
	EnemyStats stats;
	float health = 0.0f;
	float speed = 0.0f;

	TransformComponent* transform = nullptr;
	RigidBodyComponent* body = nullptr;
	TransformComponent* visual = nullptr;
	TransformComponent* armLeft = nullptr;
	TransformComponent* armRight = nullptr;
	TransformComponent* legLeft = nullptr;
	TransformComponent* legRight = nullptr;
	TransformComponent* gunTip = nullptr; //gunners only
	MeshRenderer* visor = nullptr;
	std::vector<MeshRenderer*> meshes;
	std::vector<glm::vec3> baseEmissive;

	State state = State::Chase;
	float stateTimer = 0.0f;
	float attackCooldown = 1.0f;
	float flashTimer = 0.0f;
	float staggerTimer = 0.0f;
	float yaw = 0.0f;
	float walkPhase = 0.0f;
	float strafeSign = 1.0f;
	float strafeTimer = 0.0f;
	float avoidSign = 1.0f;
	float stuckTimer = 0.0f;
	glm::vec3 stuckCheckPosition = glm::vec3(0);
	float sightTimer = 0.0f;
	bool canSeePlayer = false;
	float spawnTimer = 0.0f;
};
