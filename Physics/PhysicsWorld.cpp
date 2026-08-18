#include "PhysicsWorld.h"

PhysicsWorld::PhysicsWorld(EventBus& eb)
	: eventBus(eb) {}

Collision::CollisionFn Collision::dispatchTable[ShapeCount][ShapeCount] = {};
Collision::BroadCollisionFn Collision::broadDispatchTable[ShapeCount] = {};


//register function pointers in dispatch table
void Collision::init() {
	dispatchTable[(int)SHAPE::Sphere][(int)SHAPE::Sphere] = Collision::SphereSphere;
	dispatchTable[(int)SHAPE::Sphere][(int)SHAPE::Box] = Collision::SphereBox;
	dispatchTable[(int)SHAPE::Box][(int)SHAPE::Sphere] = Collision::BoxSphere;
	dispatchTable[(int)SHAPE::Box][(int)SHAPE::Box] = Collision::BoxBox;
	/*dispatchTable[(int)SHAPE::Capsule][(int)SHAPE::Box] = Collision::CapsuleBox;
	dispatchTable[(int)SHAPE::Box][(int)SHAPE::Capsule] = Collision::BoxCapsule;
	dispatchTable[(int)SHAPE::Capsule][(int)SHAPE::Sphere] = Collision::CapsuleSphere;
	dispatchTable[(int)SHAPE::Sphere][(int)SHAPE::Capsule] = Collision::SphereCapsule;
	dispatchTable[(int)SHAPE::Capsule][(int)SHAPE::Capsule] = Collision::CapsuleCapsule;*/

	broadDispatchTable[(int)SHAPE::Box] = Collision::BoxShape;
	broadDispatchTable[(int)SHAPE::Sphere] = Collision::SphereShape;
	//broadDispatchTable[(int)SHAPE::Capsule] = Collision::CapsuleShape;
}

bool Collision::SphereSphere(ColliderComponent& a, ColliderComponent& b, Result& r) {
	Sphere* sphereA = static_cast<Sphere*>(a.shape.get());
	Sphere* sphereB = static_cast<Sphere*>(b.shape.get());
	glm::vec3 pointA = a.GetMatrix()[3];
	glm::vec3 delta =  glm::vec3(b.GetMatrix()[3]) - pointA;
	
	float radiusSum = sphereA->radius + sphereB->radius;
	if (radiusSum * radiusSum >= glm::length2(delta)) {
		r.a = &a;
		r.b = &b;
		if (glm::length2(delta) < 1e-6f) {
			r.normal = glm::vec3(0, 1, 0);
			r.penetration = radiusSum;
		}
		else {
			r.normal = glm::normalize(delta);
			r.penetration = radiusSum - glm::length(delta);
		}
		r.point = pointA + r.normal * sphereA->radius;
		return true;
	}
	return false;
}
bool Collision::BoxSphere(ColliderComponent& a, ColliderComponent& b, Result& result) {
	Box* boxA = static_cast<Box*>(a.shape.get());
	Sphere* sphereB = static_cast<Sphere*>(b.shape.get());

	glm::mat4 matA = a.GetMatrix();
	glm::mat4 matB = b.GetMatrix();

	OBB box;
	box.center = matA[3];

	box.right = glm::normalize(matA[0]);
	box.up = glm::normalize(matA[1]);
	box.forward = glm::normalize(matA[2]);
	box.halfExtent = boxA->halfExtent;

	glm::vec3 d = glm::vec3(matB[3]) - box.center;

	float x = glm::dot(d, box.right);
	float y = glm::dot(d, box.up);
	float z = glm::dot(d, box.forward);

	float cx = glm::clamp(x, -box.halfExtent.x, box.halfExtent.x);
	float cy = glm::clamp(y, -box.halfExtent.y, box.halfExtent.y);
	float cz = glm::clamp(z, -box.halfExtent.z, box.halfExtent.z);

	glm::vec3 closest = glm::vec3(matA[3]) + box.right * cx + box.up * cy + box.forward * cz;

	glm::vec3 diff = glm::vec3(matB[3]) - closest;

	float dist2 = glm::length2(diff);
	if (dist2 < sphereB->radius * sphereB->radius) {
		result.a = &a;
		result.b = &b;
		
		float distance = sqrt(dist2);
		
		if (distance < 1e-6f) {
			result.normal = box.up;
		}
		else {
			result.normal = diff / distance;
		}
		result.penetration = sphereB->radius - distance;
		result.point = closest;
		return true;
	}

	return false;
}
bool Collision::SphereBox(ColliderComponent& a, ColliderComponent& b, Result& result) {
	bool hit = Collision::BoxSphere(b, a, result);
	if (hit) {
		std::swap(result.a, result.b);
		result.normal = -result.normal;
	}
	return hit;
}

static float ProjectRadius(Collision::OBB& obb, glm::vec3 axis) {
	return
		obb.halfExtent.x * abs(glm::dot(axis, glm::normalize(obb.right))) +
		obb.halfExtent.y * abs(glm::dot(axis, glm::normalize(obb.up))) +
		obb.halfExtent.z * abs(glm::dot(axis, glm::normalize(obb.forward)));
}

AABB Collision::SphereShape(ColliderComponent& c) {
	glm::vec3 pos = c.GetMatrix()[3];
	AABB box;
	float radius = ((Sphere*)c.shape.get())->radius;
	box.min = pos - radius;
	box.max = pos + radius;
	return box;
}
AABB Collision::BoxShape(ColliderComponent& c) {
	glm::mat4 mat = c.GetMatrix();
	glm::vec3 halfExtents = ((Box*)c.shape.get())->halfExtent;
	
	glm::vec3 right = glm::normalize(glm::vec3(mat[0]));
	glm::vec3 up = glm::normalize(glm::vec3(mat[1]));
	glm::vec3 forward = glm::normalize(glm::vec3(mat[2]));

	glm::vec3 aabbHalf;
	aabbHalf.x = abs(right.x) * halfExtents.x + abs(up.x) * halfExtents.y + abs(forward.x) * halfExtents.z;
	aabbHalf.y = abs(right.y) * halfExtents.x + abs(up.y) * halfExtents.y + abs(forward.y) * halfExtents.z;
	aabbHalf.z = abs(right.z) * halfExtents.x + abs(up.z) * halfExtents.y + abs(forward.z) * halfExtents.z;

	AABB a;
	a.min = glm::vec3(mat[3]) - aabbHalf;
	a.max = glm::vec3(mat[3]) + aabbHalf;

	return a;
}

//SHOULD CREATE OBB struct to store axis of box instead of normalizing every time -> EXPENSIVE!
bool Collision::BoxBox(ColliderComponent& a, ColliderComponent& b, Result& result) {
	Box* shapeA = static_cast<Box*>(a.shape.get());
	Box* shapeB = static_cast<Box*>(b.shape.get());

	glm::mat4 matA = a.GetMatrix();
	glm::mat4 matB = b.GetMatrix();

	OBB boxA = {
		matA[3], matA[0], matA[1], matA[2], shapeA->halfExtent
	};
	OBB boxB = {
		matB[3], matB[0], matB[1], matB[2], shapeB->halfExtent
	};	

	glm::vec3 axes[15];
	axes[0] = glm::normalize(boxA.right);
	axes[1] = glm::normalize(boxA.up);
	axes[2] = glm::normalize(boxA.forward);
	axes[3] = glm::normalize(boxB.right);
	axes[4] = glm::normalize(boxB.up);
	axes[5] = glm::normalize(boxB.forward);

	int index = 6;

	glm::vec3 delta = boxB.center - boxA.center;

	for (int i = 0; i < 3; ++i) {
		for (int j = 3; j < 6; ++j) {
			glm::vec3 axis = glm::cross(axes[i], axes[j]);

			if (glm::length2(axis) > 0.0001f) {
				axes[index++] = glm::normalize(axis);
			}
		}
	}

	float minOverlap = FLT_MAX;
	glm::vec3 bestAxis;
	for (int i = 0; i < index; ++i) {
		glm::vec3 axis = axes[i];

		float distance = abs(glm::dot(delta, axis));

		float radiusA = ProjectRadius(boxA, axis);
		float radiusB = ProjectRadius(boxB, axis);

		float overlap = radiusA + radiusB - distance;
		if (overlap < minOverlap) {
			if (overlap < 0) return false;
			minOverlap = overlap;
			bestAxis = axis;
		}
	}
	result.a = &a;
	result.b = &b;
	if (glm::dot(bestAxis, delta) > 0)
		result.normal = bestAxis;
	else
		result.normal = -bestAxis;
	result.penetration = minOverlap;
	result.point = (boxA.center + boxB.center) * 0.5f; // NEEDS TO BE  IMPROVED -> implement clipping
	return true;
}
bool Collision::AABBTest(const AABB& a, const AABB& b) {
	bool overlapX = (a.min.x <= b.max.x) && (a.max.x >= b.min.x);
	bool overlapY = (a.min.y <= b.max.y) && (a.max.y >= b.min.y);
	bool overlapZ = (a.min.z <= b.max.z) && (a.max.z >= b.min.z);
	return overlapX && overlapY && overlapZ;
}
bool Collision::Test(ColliderComponent& a, ColliderComponent& b, Result& result) {	
	auto fn = dispatchTable[(int)a.shape->GetType()][(int)b.shape->GetType()];
	if (fn) {
		return fn(a, b, result);
	}
	return false;
} 
void PhysicsWorld::BuildBroad() {
	for (auto collider : colliders) {
		collider->aabb = Collision::broadDispatchTable[(int)collider->shape->GetType()](*collider);
	}
}

void PhysicsWorld::BroadPhase() {
	broadPhase.clear();
	for (int i = 0; i < colliders.size(); ++i) {
		for (int j = i + 1; j < colliders.size(); ++j) {
			ColliderComponent* a = colliders[i];
			ColliderComponent* b = colliders[j];
			if (a->getOwner() == b->getOwner()) continue;
			if (Collision::AABBTest(a->aabb, b->aabb)) {
				broadPhase.push_back(Pair(a, b));
			}
		}
	}
}

void PhysicsWorld::Step(float dt) {
	for (RigidBodyComponent* body : bodies) {
		if (body->isStatic) continue;
		if (body->useGravity)
			body->AddForce(gravity * body->mass);
		
		Integrate(body, dt);

		body->accumulatedForce = glm::vec3(0);
		body->accumulatedTorque = glm::vec3(0);

		body->pendingImpulse = glm::vec3(0);
		body->pendingAngularImpulse = glm::vec3(0);
	}
	if (triggerBufferSwitch) triggerBuffer2.clear();
	else triggerBuffer1.clear();

	BuildBroad();
	BroadPhase();
	CollisionCheck();
	UpdateTrigger();
}
void PhysicsWorld::ResolveCollision(RigidBodyComponent* ra , RigidBodyComponent* rb, Result& result) {
	float inverseMassA = (ra && !ra->IsStatic()) ? ra->GetInverseMass() : 0.0f;
	float inverseMassB = (rb && !rb->IsStatic()) ? rb->GetInverseMass() : 0.0f;
	float totalMass = inverseMassA + inverseMassB;
	if (totalMass == 0) return; //both static
	float percent = 0.8f;
	float slop = 0.01f;
	float depth = glm::max(result.penetration - slop, 0.0f);

	glm::vec3 correction = result.normal * (depth / totalMass) * percent;
	if (ra)
		ra->Move(-correction * inverseMassA);
	if (rb)
		rb->Move(correction * inverseMassB);

	//impulse resolution
	glm::vec3 velocityA = (ra) ? ra->GetVelocity() : glm::vec3(0);
	glm::vec3 velocityB = (rb) ? rb->GetVelocity() : glm::vec3(0);

	glm::vec3 relativeVelocity = velocityB - velocityA;
	float velocityNormal = glm::dot(result.normal, relativeVelocity);

	if (velocityNormal > 0) return;
	float restitution = 0.0f; //should be a property on collider or rb
	float j = -(1 + restitution) * velocityNormal;
	j /= inverseMassA + inverseMassB;

	glm::vec3 impulse = j * result.normal;
	if (ra)
		ra->AddImpulse(-impulse);
	if (rb)
		rb->AddImpulse(impulse);
	
}
void PhysicsWorld::UpdateTrigger() {
	std::unordered_set<Pair, PairHash>* currentTrigger = triggerBufferSwitch ? &triggerBuffer2 : &triggerBuffer1;
	std::unordered_set<Pair, PairHash>* previousTrigger = triggerBufferSwitch ? &triggerBuffer1 : &triggerBuffer2;
	for (auto& pair : *currentTrigger) {
		if (previousTrigger->contains(pair))
			//stay
			continue;
		else {
			//TODO: helper function for emits
			//enter
			TriggerEnterEvent event;
			event.trigger = pair.a->isTrigger ? pair.a : pair.b;
			event.other = pair.a->isTrigger ? pair.b : pair.a;
			eventBus.Emit(event);
		}
	}

	for (auto& pair : *previousTrigger) {
		if (!currentTrigger->contains(pair)) {
			//leave
			TriggerExitEvent event;
			event.trigger = pair.a->isTrigger ? pair.a : pair.b;
			event.other = pair.a->isTrigger ? pair.b : pair.a;
			eventBus.Emit(event);
		}
	}
	triggerBufferSwitch = !triggerBufferSwitch;
}
void PhysicsWorld::HandleCollision(Result& result) {
	if (result.a->isTrigger || result.b->isTrigger) {
		TriggerEnterEvent event;
		event.trigger = result.a->isTrigger ? result.a : result.b;
		event.other = result.a->isTrigger ? result.b : result.a;
		eventBus.Emit(event);
		return;
	}
	RigidBodyComponent* ra = result.a->GetRigidBody();
	RigidBodyComponent* rb = result.b->GetRigidBody();
	if (ra == nullptr && rb == nullptr) return; //static vs static

	ResolveCollision(ra, rb, result);
}

void PhysicsWorld::CollisionCheck() {
	for (Pair& pair : broadPhase) {
		Result result;
		if (!Collision::Test(*pair.a, *pair.b, result)) continue;
		if (pair.a->isTrigger || pair.b->isTrigger) {
			if (triggerBufferSwitch) triggerBuffer2.insert(pair);
			else triggerBuffer1.insert(pair);
			continue;
		}
		HandleCollision(result);
	}
}

void PhysicsWorld::Integrate(RigidBodyComponent* body, float dt) {
	TransformComponent* transform = body->owner->GetComponent<TransformComponent>();
	//linear
	glm::vec3 acceleration = body->accumulatedForce * body->inverseMass;
	body->velocity += acceleration * dt;
	body->velocity += body->pendingImpulse * body->inverseMass;
	transform->Translate(body->velocity * dt);

	//angular
	glm::mat3 R = glm::mat3_cast(transform->GetQuat());
	glm::mat3 inverseInertiaWorld = R * body->GetInverseLocalInertiaTensor() * glm::transpose(R);

	glm::vec3 angularAcceleration = inverseInertiaWorld * body->accumulatedTorque;
	body->angularVelocity += angularAcceleration * dt;
	body->angularVelocity += body->pendingAngularImpulse * inverseInertiaWorld;
	float angularSpeed = glm::length(body->angularVelocity);
	if (angularSpeed > 0) {
		glm::vec3 axis = body->angularVelocity / angularSpeed;
		float angle = angularSpeed * dt;

		glm::quat deltaRotation = glm::angleAxis(angle, axis);
		transform->Rotate(deltaRotation);

	}
}