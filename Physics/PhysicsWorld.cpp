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
		obb.halfExtent.x * abs(glm::dot(axis, obb.right)) +
		obb.halfExtent.y * abs(glm::dot(axis, obb.up)) +
		obb.halfExtent.z * abs(glm::dot(axis, obb.forward));
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
void Collision::ClosestPointsBetweenEdges(const glm::vec3& p1, const glm::vec3& q1, const glm::vec3& p2, const glm::vec3& q2, glm::vec3& c1, glm::vec3& c2) {
	//represent each edge
	glm::vec3 d1 = q1 - p1;
	glm::vec3 d2 = q2 - p2;
	glm::vec3 r = p1 - p2;
	// squared length of edge vectors
	float a = glm::dot(d1, d1);
	float e = glm::dot(d2, d2);
	// how much of r points along d2
	float f = glm::dot(d2, r);
	//c1 = p1 + d1 * s
	//c2 = p2 + d2 * t
	// therefor solve for s and t
	float s;
	float t;

	const float epsilon = 1e-6f;

	// case when both edges degenerate into points
	if (a <= epsilon && e <= epsilon) {
		c1 = p1;
		c2 = p2;
		return;
	}
	// case when the first edge degenerates into a point
	if (a <= epsilon) {
		s = 0.0f;
		t = glm::clamp(f / e, 0.0f, 1.0f);
	}
	else {
		float c = glm::dot(d1, r);
		// case when the second edge degenerates into a point
		if (e <= epsilon) {
			t = 0.0f;
			s = glm::clamp(-c / a, 0.0f, 1.0f);
		}
		// 2 real edges case
		else {
			// how parallel are the two edges
			float b = glm::dot(d1, d2);
			float denominator = a * e - b * b;

			// parallel case
			if (denominator != 0.0f) {
				s = glm::clamp((b * f - c * e) / denominator, 0.0f, 1.0f);
			}
			else s = 0.0f;

			t = (b * s + f) / e;
			if (t < 0.0f) {
				t = 0.0f;
				s = glm::clamp(-c / a, 0.0f, 1.0f);
			}
			else if (t > 1.0f) {
				t = 1.0f;
				s = glm::clamp((b - c) / a, 0.0f, 1.0f);
			}
		}

	}
	c1 = p1 + d1 * s;
	c2 = p2 + d2 * t;

}
//sutherland–hodgman polygon clipping algorithm
std::vector<glm::vec3> Collision::ClipPolygon(const std::vector<glm::vec3>& polygon, const glm::vec3& planeNormal, float planeDistance) {
	// Clip the polygon against the plane defined by planeNormal and planeDistance
	//plane normal points to the outside
	// current distance = dot(planeNormal, point) - planeDistance
	// negative = inside the plane
	// positive = outside the plane
	//if an edge has positive then negative, then must find intersection
	std::vector<glm::vec3> output;
	if (polygon.empty()) return output;

	constexpr float epsilon = 0.0001f;

	for (size_t i = 0; i < polygon.size(); ++i) {
		glm::vec3 current = polygon[i];
		glm::vec3 next = polygon[(i + 1) % polygon.size()];

		// negative = within the plane, positive = outside the plane
		float currentDistance = glm::dot(planeNormal, current) - planeDistance;
		float nextDistance = glm::dot(planeNormal, next) - planeDistance;

		bool currentInside = currentDistance <= epsilon;
		bool nextInside = nextDistance <= epsilon;
		//no clipping -> both inside
		if (currentInside && nextInside) {
			output.push_back(next);
		}
		// current inside, next outside -> clip and fine intersect
		else if (currentInside && !nextInside) {
			// proportion that current point is inside the plane out of the total distance of both points
			float t = currentDistance / (currentDistance - nextDistance);
			// intersection point is current, moved towards next by proportion
			output.push_back(current + t * (next - current));
		}
		else if (!currentInside && nextInside) {
		    // same idea, but also keep next
			float t = currentDistance / (currentDistance - nextDistance);

			output.push_back(current + t * (next - current));
			output.push_back(next);
		}
	}
	return output;
}
std::vector<glm::vec3> Collision::GetFaceVertices(const OBB& box, const glm::vec3& normal) {
	//find the vertices of the face best facing the normal
	glm::vec3 axes[3] = {
		box.right,
		box.up,
		box.forward
	};

	float extents[3] = {
		box.halfExtent.x,
		box.halfExtent.y,
		box.halfExtent.z
	};

	int faceAxis = 0;
	float bestDot = -FLT_MAX;

	for (int i = 0; i < 3; ++i) {
		float d = glm::abs(glm::dot(axes[i], normal));
		if (d > bestDot) {
			bestDot = d;
			faceAxis = i;
		}
	}

	glm::vec3 faceNormal = axes[faceAxis];

	if (glm::dot(faceNormal, normal) < 0.0f) {
		faceNormal = -faceNormal;
	}

	glm::vec3 faceCenter = box.center + faceNormal * extents[faceAxis];

	int axis1 = (faceAxis + 1) % 3;
	int axis2 = (faceAxis + 2) % 3;

	glm::vec3 e1 = axes[axis1] * extents[axis1];
	glm::vec3 e2 = axes[axis2] * extents[axis2];
	return {
		faceCenter + e1 + e2,
		faceCenter + e1 - e2,
		faceCenter - e1 - e2,
		faceCenter - e1 + e2
	};
}
std::vector<glm::vec3> Collision::GenerateManifold(const OBB& box1, const OBB& box2, const glm::vec3& normal, int bestAxisIndex) {
	//pick reference
	// referece -> face whose boundries we clip against
	// incident -> face that will be clipped
	const OBB* reference;
	const OBB* incident;
	glm::vec3 normalForReference;
	
	int referenceAxis;
	if (bestAxisIndex < 3) {
		reference = &box1;
		incident = &box2;
		referenceAxis = bestAxisIndex;
		normalForReference = normal;
	} 
	else {
		reference = &box2;
		incident = &box1;
		referenceAxis = bestAxisIndex - 3;
		normalForReference = -normal;
	}

	glm::vec3 referenceAxes[3] = {
		reference->right,
		reference->up,
		reference->forward
	};
	float referenceExtents[3] = {
		reference->halfExtent.x,
		reference->halfExtent.y,
		reference->halfExtent.z
	};

	glm::vec3 faceNormal = referenceAxes[referenceAxis];
	if (glm::dot(faceNormal, normalForReference) < 0.0f) faceNormal = -faceNormal;

	glm::vec3 faceCenter = reference->center + faceNormal * referenceExtents[referenceAxis];
	//grab vertices of that face
	std::vector<glm::vec3> contacts = GetFaceVertices(*incident, -faceNormal);
	
	int axis1 = (referenceAxis + 1) % 3;
	int axis2 = (referenceAxis + 2) % 3;

	glm::vec3 sideAxis1 = referenceAxes[axis1];
	glm::vec3 sideAxis2 = referenceAxes[axis2];

	float extent1 = referenceExtents[axis1];
	float extent2 = referenceExtents[axis2];

	{
		glm::vec3 planeNormal = sideAxis1;

		glm::vec3 planePoint = faceCenter + sideAxis1 * extent1;

		float planeDistance = glm::dot(planeNormal, planePoint);

		contacts = ClipPolygon(contacts, planeNormal, planeDistance);
	}
	{
		glm::vec3 planeNormal = -sideAxis1;

		glm::vec3 planePoint = faceCenter - sideAxis1 * extent1;

		float planeDistance = glm::dot(planeNormal, planePoint);

		contacts = ClipPolygon(contacts, planeNormal, planeDistance);
	}
	{
		glm::vec3 planeNormal = sideAxis2;

		glm::vec3 planePoint = faceCenter + sideAxis2 * extent2;

		float planeDistance = glm::dot(planeNormal, planePoint);

		contacts = ClipPolygon(contacts, planeNormal, planeDistance);
	}
	{
		glm::vec3 planeNormal = -sideAxis2;

		glm::vec3 planePoint = faceCenter - sideAxis2 * extent2;

		float planeDistance = glm::dot(planeNormal, planePoint);

		contacts = ClipPolygon(contacts, planeNormal, planeDistance);
	}

	float referencePlane = glm::dot(faceNormal, faceCenter);
	std::vector<glm::vec3> manifold;
	constexpr float epsilon = 0.01f;
	for (glm::vec3& point : contacts) {
		float distance = glm::dot(faceNormal, point) - referencePlane;

		if (distance <= epsilon) {
			point -= faceNormal * distance;
			manifold.push_back(point);
		}
	}


		
	return manifold;
}

glm::vec3 Collision::FurthestPoint(const OBB& box, const glm::vec3& direction) {
	glm::vec3 point = box.center;
	point += box.right * (glm::dot(box.right, direction) >= 0.0f ? box.halfExtent.x : -box.halfExtent.x);
	point += box.up * (glm::dot(box.up, direction) >= 0.0f ? box.halfExtent.y : -box.halfExtent.y);
	point += box.forward * (glm::dot(box.forward, direction) >= 0.0f ? box.halfExtent.z : -box.halfExtent.z);
	return point;
}
struct SATAxis {
	glm::vec3 axis;
	int type;
	int axisA;
	int axisB;
};
bool Collision::BoxBox(ColliderComponent& a, ColliderComponent& b, Result& result) {
	Box* shapeA = static_cast<Box*>(a.shape.get());
	Box* shapeB = static_cast<Box*>(b.shape.get());

	glm::mat4 matA = a.GetMatrix();
	glm::mat4 matB = b.GetMatrix();

	OBB boxA = {
		matA[3], glm::normalize(matA[0]), glm::normalize(matA[1]), glm::normalize(matA[2]), shapeA->halfExtent
	};
	OBB boxB = {
		matB[3], glm::normalize(matB[0]), glm::normalize(matB[1]), glm::normalize(matB[2]), shapeB->halfExtent
	};	
	SATAxis axes[15];
	axes[0] = { boxA.right,   0, 0, -1 };
	axes[1] = { boxA.up,      0, 1, -1 };
	axes[2] = { boxA.forward, 0, 2, -1 };

	axes[3] = { boxB.right,   1, -1, 0 };
	axes[4] = { boxB.up,	  1, -1, 1 };
	axes[5] = { boxB.forward, 1, -1, 2 };

	int index = 6;

	glm::vec3 delta = boxB.center - boxA.center;
	for (int i = 0; i < 3; ++i) {
		for (int j = 3; j < 6; ++j) {
			glm::vec3 axis = glm::cross(axes[i].axis, axes[j].axis);

			if (glm::length2(axis) > 0.0001f) {
				axes[index++] = { glm::normalize(axis), 2, i, j -3};
			}
		}
	}

	float minOverlap = FLT_MAX;
	int bestAxisIndex = -1;
	for (int i = 0; i < index; ++i) {
		glm::vec3 axis = axes[i].axis;

		float distance = abs(glm::dot(delta, axis));

		float radiusA = ProjectRadius(boxA, axis);
		float radiusB = ProjectRadius(boxB, axis);

		float overlap = radiusA + radiusB - distance;
		if (overlap < minOverlap) {
			if (overlap < 0) return false;
			minOverlap = overlap;
			bestAxisIndex = i;
		}
	}
	result.a = &a;
	result.b = &b;
	if (glm::dot(axes[bestAxisIndex].axis, delta) > 0)
		result.normal = axes[bestAxisIndex].axis;
	else
		result.normal = -axes[bestAxisIndex].axis;
	result.penetration = minOverlap;


	if (bestAxisIndex < 6) {


		//Face Contact -> face clipping
		auto contacts = GenerateManifold(boxA, boxB, result.normal, bestAxisIndex);
		
		if (contacts.empty()) {
			result.point = (FurthestPoint(boxA, result.normal) + FurthestPoint(boxB, -result.normal)) * 0.5f;
		}
		else {
			result.point = glm::vec3(0);
			for (const glm::vec3& p : contacts) {
				result.point += p;
			}
			result.point /= static_cast<float>(contacts.size());
		}
	}
	else {
		//edge - edge
		SATAxis& sat = axes[bestAxisIndex];

		glm::vec3 axesA[3] = {
			boxA.right, boxA.up, boxA.forward
		};
		glm::vec3 axesB[3] = {
			boxB.right, boxB.up, boxB.forward
		};

		float extentsA[3] = {
			boxA.halfExtent.x,
			boxA.halfExtent.y,
			boxA.halfExtent.z
		};
		float extentsB[3] = {
			boxB.halfExtent.x,
			boxB.halfExtent.y,
			boxB.halfExtent.z
		};

		glm::vec3 edgeCenterA = boxA.center;
		for (int i = 0; i < 3; ++i) {
			if (i == sat.axisA) continue;
			float sign = glm::dot(axesA[i], result.normal) > 0.0f ? 1.0f : -1.0f;
			edgeCenterA += axesA[i] * extentsA[i] * sign;
		}
		glm::vec3 edgeDirA = axesA[sat.axisA];
		glm::vec3 edgeA0 = edgeCenterA - edgeDirA * extentsA[sat.axisA];
		glm::vec3 edgeA1 = edgeCenterA + edgeDirA * extentsA[sat.axisA];

		glm::vec3 edgeCenterB = boxB.center;
		for (int i = 0; i < 3; ++i) {
			if (i == sat.axisB) continue;
			float sign = glm::dot(axesB[i], -result.normal) >= 0.0f ? 1.0f : -1.0f;
			edgeCenterB += axesB[i] * extentsB[i] * sign;
		}
		glm::vec3 edgeDirB = axesB[sat.axisB];
		glm::vec3 edgeB0 = edgeCenterB - edgeDirB * extentsB[sat.axisB];
		glm::vec3 edgeB1 = edgeCenterB + edgeDirB * extentsB[sat.axisB];

		glm::vec3 pointA;
		glm::vec3 pointB;

		ClosestPointsBetweenEdges(edgeA0, edgeA1, edgeB0, edgeB1, pointA, pointB);
		result.point = (pointA + pointB) * 0.5f;
	}
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
		if (!body->isStatic) {
			if (body->useGravity)
				body->AddForce(gravity * body->mass);

			Integrate(body, dt);
		}

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

	//impulse resolution
	glm::vec3 rA = (ra) ? result.point - ra->GetCenterMass() : glm::vec3(0);
	glm::vec3 rB = (rb) ? result.point - rb->GetCenterMass() : glm::vec3(0);

	glm::vec3 velocityA = (ra) ? ra->GetVelocity() + glm::cross(ra->GetAngularVelocity(), rA) : glm::vec3(0);
	glm::vec3 velocityB = (rb) ? rb->GetVelocity() + glm::cross(rb->GetAngularVelocity(), rB) : glm::vec3(0);

	glm::vec3 relativeVelocity = velocityB - velocityA;
	float velocityNormal = glm::dot(result.normal, relativeVelocity);

	if (velocityNormal > 0) return;

	glm::mat3 inverseInertiaA(0.0f);
	glm::mat3 inverseInertiaB(0.0f);

	if (ra && !ra->IsStatic()) {
		glm::mat3 R = glm::mat3_cast(ra->owner->GetComponent<TransformComponent>()->GetQuat());
		inverseInertiaA = R * ra->GetInverseLocalInertiaTensor() * glm::transpose(R);
	}
	if (rb && !rb->IsStatic()) {
		glm::mat3 R = glm::mat3_cast(rb->owner->GetComponent<TransformComponent>()->GetQuat());
		inverseInertiaB = R * rb->GetInverseLocalInertiaTensor() * glm::transpose(R);
	}

	glm::vec3 rnA = glm::cross(rA, result.normal);
	glm::vec3 rnB = glm::cross(rB, result.normal);

	float rotationalA = glm::dot(result.normal, glm::cross(inverseInertiaA * rnA, rA));
	float rotationalB = glm::dot(result.normal, glm::cross(inverseInertiaB * rnB, rB));

	float denominator = inverseMassA + inverseMassB + rotationalA + rotationalB;
	if (denominator <= 1e-6f) return;
	float restitution = 0.0f; //should be a property on collider or rb
	float j = -(1.0f + restitution) * velocityNormal / denominator;
	glm::vec3 impulse = j * result.normal;
	if (ra && !ra->IsStatic()) {
		ra->velocity += (-impulse) * ra->inverseMass;
		ra->angularVelocity += inverseInertiaA * glm::cross(rA, -impulse);
	}
	if (rb && !rb->IsStatic()) {
		rb->velocity += impulse * rb->inverseMass;
		rb->angularVelocity += inverseInertiaB * glm::cross(rB, impulse);
	}

	float percent = 0.8f;
	float slop = 0.01f;
	float depth = glm::max(result.penetration - slop, 0.0f);

	glm::vec3 correction = result.normal * (depth / totalMass) * percent;
	if (ra)
		ra->Move(-correction * inverseMassA);
	if (rb)
		rb->Move(correction * inverseMassB);
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
	body->angularVelocity += inverseInertiaWorld * body->pendingAngularImpulse;

	body->ApplyConstraint();

	body->angularVelocity *= 0.995f;
	float angularSpeed = glm::length(body->angularVelocity);
	if (angularSpeed < 1e-5f) {
		body->angularVelocity = glm::vec3(0.0f);
		return;
	}
	if (angularSpeed > 0) {
		glm::vec3 axis = body->angularVelocity / angularSpeed;
		float angle = angularSpeed * dt;

		glm::quat deltaRotation = glm::angleAxis(angle, axis);
		transform->Rotate(deltaRotation);

	}
}