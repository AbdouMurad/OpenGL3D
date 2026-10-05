#include "Enemy.h"
#include "FPSGame.h"

static EnemyStats StatsFor(EnemyType type) {
	switch (type) {
	case EnemyType::Gunner:
		//keeps its distance and lobs plasma bolts, attackRange is how far away it likes to stand
		return { 90.0f, 3.4f, 10.0f, 12.0f, 2.0f, 0.6f, 1.0f, 80.0f, 150,
			{ 0.22f, 0.38f, 0.75f }, { 0.3f, 2.5f, 4.0f } };
	case EnemyType::Brute:
		//slow, huge and hits very hard
		return { 340.0f, 2.9f, 30.0f, 2.5f, 1.6f, 0.7f, 1.45f, 220.0f, 400,
			{ 0.3f, 0.3f, 0.27f }, { 5.0f, 0.4f, 0.2f } };
	default:
		//rushes in and swings
		return { 70.0f, 5.3f, 12.0f, 1.8f, 1.0f, 0.4f, 1.0f, 70.0f, 100,
			{ 0.75f, 0.22f, 0.16f }, { 4.0f, 2.2f, 0.3f } };
	}
}

Enemy::Enemy(FPSGame* g, EnemyType t, float healthScale, float speedScale)
	: game(g), type(t), stats(StatsFor(t)) {
	stats.health *= healthScale;
	stats.speed *= speedScale;
	health = stats.health;
	speed = stats.speed;
	attackCooldown = RandomFloat(0.5f, 1.5f);
	strafeSign = RandomFloat(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
	avoidSign = RandomFloat(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
}

Enemy& Enemy::Spawn(FPSGame& game, EnemyType type, const glm::vec3& feetPosition, float healthScale, float speedScale) {
	GameObject& root = game.scene.CreateObject();
	Enemy* enemy = root.AddComponent<Enemy>(&game, type, healthScale, speedScale);
	//the root sits at the middle of the body, slightly above the floor so it settles onto it
	root.GetComponent<TransformComponent>()->SetPosition(feetPosition + glm::vec3(0.0f, 0.9f * enemy->stats.scale + 0.02f, 0.0f));
	return *enemy;
}

void Enemy::Start() {
	Build();
}

TransformComponent* Enemy::AddPart(TransformComponent* parent, const glm::vec3& position, const glm::vec3& size, const glm::vec3& tint, const glm::vec3& emissive) {
	GameObject& part = CreatePart(game->scene, game->cubeModel, parent, position, size, tint, emissive);
	meshes.push_back(part.GetComponent<MeshRenderer>());
	return part.GetComponent<TransformComponent>();
}

TransformComponent* Enemy::AddPivot(TransformComponent* parent, const glm::vec3& position) {
	GameObject& pivot = game->scene.CreateObject();
	TransformComponent* t = pivot.GetComponent<TransformComponent>();
	t->parent = parent;
	t->SetPosition(position);
	return t;
}

void Enemy::Build() {
	const float s = stats.scale;
	transform = owner->GetComponent<TransformComponent>();

	body = owner->AddComponent<RigidBodyComponent>();
	body->SetMass(stats.mass);
	body->SetConstraint(RigidBodyConstraints::FreezeRotation);

	//a box from the feet to the shoulders and a sphere for the head, so headshots can be told apart
	ColliderComponent* bodyCollider = owner->AddComponent<ColliderComponent>(SHAPE::Box);
	Box* box = static_cast<Box*>(bodyCollider->shape.get());
	box->halfExtent = glm::vec3(0.34f, 0.7f, 0.34f) * s;
	bodyCollider->localOffset.position = glm::vec3(0.0f, -0.2f * s, 0.0f);
	bodyCollider->friction = 0.0f; //movement is driven by velocity, friction would only fight it
	bodyCollider->SetID("body");

	ColliderComponent* headCollider = owner->AddComponent<ColliderComponent>(SHAPE::Sphere);
	static_cast<Sphere*>(headCollider->shape.get())->radius = 0.27f * s;
	headCollider->localOffset.position = glm::vec3(0.0f, 0.72f * s, 0.0f);
	headCollider->friction = 0.0f;
	headCollider->SetID("head");

	//each collider's Start set the inertia from its own shape, the last one (the head) won. Use the body
	body->SetLocalInertiaTensor(box);

	GameObject& visualObject = game->scene.CreateObject();
	visual = visualObject.GetComponent<TransformComponent>();
	visual->parent = transform;
	visual->SetSize(glm::vec3(0.01f));

	const glm::vec3 color = stats.color;
	const glm::vec3 dark = color * 0.4f + glm::vec3(0.04f);
	const glm::vec3 metal(0.22f, 0.22f, 0.25f);
	const glm::vec3 glass(0.08f);

	AddPart(visual, glm::vec3(0.0f, 0.1f, 0.0f) * s, glm::vec3(0.62f, 0.7f, 0.38f) * s, color);   //torso
	AddPart(visual, glm::vec3(0.0f, -0.3f, 0.0f) * s, glm::vec3(0.5f, 0.16f, 0.32f) * s, metal);  //waist
	AddPart(visual, glm::vec3(0.0f, 0.72f, 0.0f) * s, glm::vec3(0.44f, 0.4f, 0.42f) * s, metal);  //head
	TransformComponent* visorTransform = AddPart(visual, glm::vec3(0.0f, 0.75f, -0.215f) * s, glm::vec3(0.36f, 0.08f, 0.02f) * s, glass, stats.visorColor);
	visor = visorTransform->getOwner()->GetComponent<MeshRenderer>();
	AddPart(visual, glm::vec3(0.0f, 0.25f, -0.195f) * s, glm::vec3(0.2f, 0.12f, 0.02f) * s, glass, stats.visorColor * 0.5f); //chest light

	armLeft = AddPivot(visual, glm::vec3(-0.42f, 0.4f, 0.0f) * s);
	armRight = AddPivot(visual, glm::vec3(0.42f, 0.4f, 0.0f) * s);
	AddPart(armLeft, glm::vec3(0.0f, -0.3f, 0.0f) * s, glm::vec3(0.18f, 0.66f, 0.2f) * s, dark);
	AddPart(armRight, glm::vec3(0.0f, -0.3f, 0.0f) * s, glm::vec3(0.18f, 0.66f, 0.2f) * s, dark);

	legLeft = AddPivot(visual, glm::vec3(-0.16f, -0.36f, 0.0f) * s);
	legRight = AddPivot(visual, glm::vec3(0.16f, -0.36f, 0.0f) * s);
	AddPart(legLeft, glm::vec3(0.0f, -0.27f, 0.0f) * s, glm::vec3(0.22f, 0.54f, 0.26f) * s, dark);
	AddPart(legRight, glm::vec3(0.0f, -0.27f, 0.0f) * s, glm::vec3(0.22f, 0.54f, 0.26f) * s, dark);

	if (type == EnemyType::Gunner) {
		//a plasma cannon where the right hand would be
		AddPart(armRight, glm::vec3(0.0f, -0.66f, 0.0f) * s, glm::vec3(0.15f, 0.3f, 0.15f) * s, metal);
		AddPart(armRight, glm::vec3(0.0f, -0.82f, 0.0f) * s, glm::vec3(0.1f, 0.04f, 0.1f) * s, glass, stats.visorColor);
		gunTip = AddPivot(armRight, glm::vec3(0.0f, -0.9f, 0.0f) * s);
	}
	if (type == EnemyType::Brute) {
		AddPart(visual, glm::vec3(-0.44f, 0.5f, 0.0f) * s, glm::vec3(0.34f, 0.16f, 0.42f) * s, color * 0.8f); //shoulder plates
		AddPart(visual, glm::vec3(0.44f, 0.5f, 0.0f) * s, glm::vec3(0.34f, 0.16f, 0.42f) * s, color * 0.8f);
		AddPart(armLeft, glm::vec3(0.0f, -0.68f, 0.0f) * s, glm::vec3(0.28f) * s, metal); //fists
		AddPart(armRight, glm::vec3(0.0f, -0.68f, 0.0f) * s, glm::vec3(0.28f) * s, metal);
	}

	for (MeshRenderer* mesh : meshes) baseEmissive.push_back(mesh->emissive);
}

glm::vec3 Enemy::GetPosition() const {
	return transform->GetPosition();
}

bool Enemy::TakeDamage(float amount, const glm::vec3& point, const glm::vec3& direction) {
	if (IsDead()) return false;
	health -= amount;
	flashTimer = 0.05f;
	if (health <= 0.0f) {
		Die(point, direction);
		return true;
	}
	//flinch back a little, brutes barely notice
	glm::vec3 flat = glm::vec3(direction.x, 0.0f, direction.z);
	float knockback = (type == EnemyType::Brute ? 0.008f : 0.03f) * amount;
	body->AddImpulse(flat * stats.mass * knockback);
	if (type != EnemyType::Brute) staggerTimer = 0.1f;
	return false;
}

void Enemy::Push(const glm::vec3& point, const glm::vec3& impulse, float spin) {
	body->AddImpulse(impulse);
	if (IsDead()) body->AddAngularImpulse(glm::cross(point - body->GetCenterMass(), impulse) * spin);
}

void Enemy::Die(const glm::vec3& point, const glm::vec3& direction) {
	state = State::Dead;
	stateTimer = 0.0f;
	health = 0.0f;

	//hand it over to the physics world and let it topple
	body->SetConstraint(RigidBodyConstraints::None);
	body->linearDamping = 0.3f;
	body->angularDamping = 2.0f;
	for (ColliderComponent* collider : owner->GetComponents<ColliderComponent>()) collider->friction = 0.8f;

	glm::vec3 flat = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z) + glm::vec3(0.0f, 0.001f, 0.0f));
	Push(point, (flat * 4.0f + glm::vec3(0.0f, 1.5f, 0.0f)) * stats.mass, 0.25f);

	for (MeshRenderer* mesh : meshes) mesh->tint *= 0.45f;
}

bool Enemy::IsStaticObstacle(ColliderComponent* collider) const {
	GameObject* hitObject = collider->getOwner();
	return hitObject != &game->GetPlayerObject() && !hitObject->GetComponent<Enemy>();
}

bool Enemy::HasLineOfSight() {
	Player& player = game->GetPlayer();
	glm::vec3 eye = GetPosition() + glm::vec3(0.0f, 0.72f * stats.scale, 0.0f);
	glm::vec3 delta = player.GetEyePosition() - eye;
	float distance = glm::length(delta);
	if (distance < 1e-3f) return true;

	RaycastHit hit;
	if (!game->scene.Raycast(eye, delta / distance, distance, hit, owner)) return true;
	//other robots don't block the view, only walls and cover do
	return !IsStaticObstacle(hit.collider);
}

glm::vec3 Enemy::GunnerMovement(const glm::vec3& direction, float distance, float dt) {
	strafeTimer -= dt;
	if (strafeTimer <= 0.0f) {
		strafeSign = RandomFloat(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
		strafeTimer = RandomFloat(1.2f, 2.8f);
	}
	glm::vec3 strafe = glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)) * strafeSign;

	if (!canSeePlayer || distance > stats.attackRange + 6.0f) return direction;
	if (distance < stats.attackRange - 5.0f) return glm::normalize(-direction + strafe * 0.5f);
	return strafe * 0.7f;
}

glm::vec3 Enemy::Steer(const glm::vec3& desired) {
	float magnitude = glm::length(desired);
	if (magnitude < 0.01f) return glm::vec3(0.0f);
	glm::vec3 position = GetPosition();
	glm::vec3 direction = desired / magnitude;

	//keep some room from other robots so they don't pile into one blob
	glm::vec3 separation(0.0f);
	for (Enemy* other : game->GetEnemies()) {
		if (other == this || other->IsDead()) continue;
		glm::vec3 away = position - other->GetPosition();
		away.y = 0.0f;
		float distance = glm::length(away);
		float room = 0.9f * (stats.scale + other->stats.scale);
		if (distance > 1e-3f && distance < room) separation += away / distance * (1.0f - distance / room);
	}
	glm::vec3 combined = direction + separation * 1.5f;
	if (glm::length2(combined) > 1e-4f) direction = glm::normalize(combined);

	//slide along walls and cover instead of walking into them, probing at waist height so low cover counts
	RaycastHit hit;
	glm::vec3 waist = position - glm::vec3(0.0f, 0.3f * stats.scale, 0.0f);
	if (game->scene.Raycast(waist, direction, 0.6f * stats.scale + 1.0f, hit, owner) && IsStaticObstacle(hit.collider)) {
		glm::vec3 normal(hit.normal.x, 0.0f, hit.normal.z);
		if (glm::length2(normal) > 1e-4f) {
			normal = glm::normalize(normal);
			glm::vec3 slide = direction - normal * glm::dot(direction, normal);
			//walking straight into it: pick a side and stick with it
			if (glm::length(slide) < 0.35f) slide = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), normal) * avoidSign;
			direction = glm::normalize(slide);
		}
	}
	return direction * magnitude;
}

void Enemy::Attack() {
	Player& player = game->GetPlayer();
	if (player.IsDead()) return;
	glm::vec3 position = GetPosition();

	if (type == EnemyType::Gunner) {
		const float projectileSpeed = 19.0f;
		glm::vec3 muzzle = gunTip->GetPosition();
		glm::vec3 target = player.GetPosition() + glm::vec3(0.0f, 0.3f, 0.0f);
		//lead the target a bit: perfect prediction is unfair, none makes strafing pointless
		glm::vec3 playerVelocity = player.GetVelocity();
		playerVelocity.y = 0.0f;
		target += playerVelocity * (glm::distance(muzzle, target) / projectileSpeed) * 0.5f;
		glm::vec3 direction = glm::normalize(target - muzzle + RandomDirection() * 0.35f);
		game->SpawnProjectile(muzzle, direction * projectileSpeed, stats.damage, owner);
		return;
	}

	//melee lands if the player is still in reach when the swing comes down
	glm::vec3 toPlayer = player.GetPosition() - position;
	float verticalGap = std::abs(toPlayer.y);
	toPlayer.y = 0.0f;
	if (glm::length(toPlayer) <= stats.attackRange + 0.5f && verticalGap < 1.6f * stats.scale) {
		player.TakeDamage(stats.damage, position);
	}
}

void Enemy::Update(float dt) {
	stateTimer += dt;
	flashTimer -= dt;
	spawnTimer += dt;

	if (IsDead()) {
		Animate(dt);
		return;
	}

	glm::vec3 position = GetPosition();
	//knocked out of the arena somehow
	if (position.y < -20.0f) {
		Die(position, glm::vec3(0.0f, -1.0f, 0.0f));
		return;
	}

	Player& player = game->GetPlayer();
	glm::vec3 toPlayer = player.GetPosition() - position;
	toPlayer.y = 0.0f;
	float distance = glm::length(toPlayer);
	glm::vec3 direction = distance > 1e-3f ? toPlayer / distance : glm::vec3(0.0f, 0.0f, -1.0f);
	bool playerAlive = !player.IsDead();

	attackCooldown -= dt;
	staggerTimer -= dt;
	sightTimer -= dt;
	if (sightTimer <= 0.0f) {
		canSeePlayer = HasLineOfSight();
		sightTimer = RandomFloat(0.15f, 0.3f);
	}

	bool melee = type != EnemyType::Gunner;
	glm::vec3 desired(0.0f);
	switch (state) {
	case State::Chase:
		if (!playerAlive) break;
		if (melee) {
			desired = direction;
			if (distance < stats.attackRange && attackCooldown <= 0.0f) {
				state = State::Windup;
				stateTimer = 0.0f;
			}
		}
		else {
			desired = GunnerMovement(direction, distance, dt);
			if (canSeePlayer && attackCooldown <= 0.0f && distance < 35.0f) {
				state = State::Windup;
				stateTimer = 0.0f;
			}
		}
		break;
	case State::Windup:
		//creep in so backing off at walking pace doesn't always escape the swing
		if (melee && distance > stats.attackRange * 0.6f) desired = direction * 0.35f;
		if (!melee) desired = GunnerMovement(direction, distance, dt) * 0.3f;
		if (stateTimer >= stats.windup) {
			Attack();
			state = State::Recover;
			stateTimer = 0.0f;
		}
		break;
	case State::Recover:
		if (!melee) desired = GunnerMovement(direction, distance, dt);
		if (stateTimer >= (melee ? 0.45f : 0.3f)) {
			state = State::Chase;
			stateTimer = 0.0f;
			attackCooldown = stats.attackCooldown * RandomFloat(0.8f, 1.25f);
		}
		break;
	default:
		break;
	}

	//turn to face the player, slower while winding up so circling them is a real option
	float turnRate = state == State::Windup ? 3.5f : 8.0f;
	yaw += glm::clamp(AngleDelta(yaw, YawTowards(direction)), -turnRate * dt, turnRate * dt);
	visual->SetRotation(glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f)));

	if (staggerTimer <= 0.0f) {
		glm::vec3 velocity = body->GetVelocity();
		glm::vec3 horizontal(velocity.x, 0.0f, velocity.z);
		horizontal = MoveTowards(horizontal, Steer(desired) * speed, 30.0f * dt);
		body->SetVelocity(glm::vec3(horizontal.x, velocity.y, horizontal.z));
	}

	//wedged against something the steering can't see: try going round the other way
	stuckTimer += dt;
	if (stuckTimer > 1.2f) {
		if (glm::length(desired) > 0.5f && glm::distance(position, stuckCheckPosition) < 0.4f) avoidSign = -avoidSign;
		stuckCheckPosition = position;
		stuckTimer = 0.0f;
	}

	Animate(dt);
}

void Enemy::Animate(float dt) {
	//grow in when spawning, shrink away a while after dying
	float size = std::min(spawnTimer / 0.35f, 1.0f);
	if (IsDead() && stateTimer > 3.0f) size = 1.0f - (stateTimer - 3.0f);
	visual->SetSize(glm::vec3(std::max(size, 0.01f)));

	for (size_t i = 0; i < meshes.size(); ++i) {
		glm::vec3 emissive = baseEmissive[i];
		if (IsDead()) emissive = glm::vec3(0.0f);
		//lights flare up before an attack, the cue to get out of the way
		else if (state == State::Windup) emissive *= 1.0f + 2.5f * (stateTimer / stats.windup);
		if (flashTimer > 0.0f) emissive += glm::vec3(0.45f, 0.4f, 0.35f);
		meshes[i]->emissive = emissive;
	}
	if (IsDead()) return;

	glm::vec3 velocity = body->GetVelocity();
	float moveSpeed = glm::length(glm::vec2(velocity.x, velocity.z));
	walkPhase += moveSpeed * dt * 2.6f / stats.scale;
	float stride = std::min(moveSpeed / std::max(speed, 0.1f), 1.0f);
	float swing = std::sin(walkPhase) * 35.0f * stride;
	legLeft->SetRotation(glm::vec3(swing, 0.0f, 0.0f));
	legRight->SetRotation(glm::vec3(-swing, 0.0f, 0.0f));

	//arms hang down, rotating around +X swings them forward and up
	float leftArm = -swing * 0.8f;
	float rightArm = swing * 0.8f;
	if (type != EnemyType::Gunner) {
		if (state == State::Windup) {
			float t = stateTimer / stats.windup;
			leftArm = rightArm = 160.0f * t * (2.0f - t);
		}
		else if (state == State::Recover) {
			float t = std::min(stateTimer / 0.12f, 1.0f);
			leftArm = rightArm = glm::mix(160.0f, 20.0f, t);
		}
	}
	else if (state != State::Chase || canSeePlayer) {
		rightArm = 90.0f;
		if (state == State::Recover) rightArm += 25.0f * (1.0f - std::min(stateTimer / 0.3f, 1.0f));
	}
	armLeft->SetRotation(glm::vec3(leftArm, 0.0f, 0.0f));
	armRight->SetRotation(glm::vec3(rightArm, 0.0f, 0.0f));
}
