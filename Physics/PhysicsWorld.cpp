#include "PhysicsWorld.h"

#include "Render/DebugRender.h"

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
	
	float radiusSum = sphereA->GetRadius() + sphereB->GetRadius();
	if (radiusSum * radiusSum >= glm::length2(delta)) {
		r.a = &a;
		r.b = &b;
		float penetration;
		if (glm::length2(delta) < 1e-6f) {
			r.normal = glm::vec3(0, 1, 0);
			penetration = radiusSum;
		}
		else {
			r.normal = glm::normalize(delta);
			penetration = radiusSum - glm::length(delta);
		}
		r.AddPoint(pointA + r.normal * sphereA->GetRadius(), penetration);
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
	box.halfExtent = boxA->GetHalfExtent();

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
	const float radius = sphereB->GetRadius();
	const float radius2 = radius * radius;
	constexpr float epsilon = 1e-6f;
	if (dist2 <= radius2 + epsilon) {
		result.a = &a;
		result.b = &b;
		
		float distance = sqrt(dist2);
		
		if (distance < epsilon) {
			//how deep contact is from each face (gives exit face)
			float dx = box.halfExtent.x - abs(x);
			float dy = box.halfExtent.y - abs(y);
			float dz = box.halfExtent.z - abs(z);

			float penetration;
			if (dx <= dy && dx <= dz) {
				result.normal = (x >= 0.0f) ? box.right : -box.right;
				penetration = radius + dx;
			}
			else if (dy <= dz) {
				result.normal = (y >= 0.0f) ? box.up : -box.up;
				penetration = radius + dy;
			}
			else {
				result.normal = (z >= 0.0f) ? box.forward : -box.forward;
				penetration = radius + dz;
			}
			result.AddPoint(glm::vec3(matB[3]) - result.normal * radius, penetration);
		}
		else {
			result.normal = diff / distance;
			result.AddPoint(closest, radius - distance);
		}
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
	float radius = ((Sphere*)c.shape.get())->GetRadius();
	box.min = pos - radius;
	box.max = pos + radius;
	return box;
}
AABB Collision::BoxShape(ColliderComponent& c) {
	glm::mat4 mat = c.GetMatrix();
	glm::vec3 halfExtents = ((Box*)c.shape.get())->GetHalfExtent();
	
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
std::vector<ContactPoint> Collision::GenerateManifold(const OBB& box1, const OBB& box2, const glm::vec3& normal, int bestAxisIndex) {
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
	std::vector<ContactPoint> manifold;
	constexpr float epsilon = 0.01f;
	for (glm::vec3& point : contacts) {
		//negative = incident corner is below the reference face (overlapping)
		float distance = glm::dot(faceNormal, point) - referencePlane;

		if (distance <= epsilon) {
			point -= faceNormal * distance;
			//each corner keeps its own depth (a tilted box is deeper on one side)
			manifold.push_back({ point, glm::max(-distance, 0.0f) });
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
		matA[3], glm::normalize(matA[0]), glm::normalize(matA[1]), glm::normalize(matA[2]), shapeA->GetHalfExtent()
	};
	OBB boxB = {
		matB[3], glm::normalize(matB[0]), glm::normalize(matB[1]), glm::normalize(matB[2]), shapeB->GetHalfExtent()
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


	if (bestAxisIndex < 6) {


		//Face Contact -> face clipping
		auto contacts = GenerateManifold(boxA, boxB, result.normal, bestAxisIndex);
		
		if (contacts.empty()) {
			result.AddPoint((FurthestPoint(boxA, result.normal) + FurthestPoint(boxB, -result.normal)) * 0.5f, minOverlap);
		}
		else {
			//every corner of the touching area is its own contact point
			for (const ContactPoint& point : contacts) {
				result.AddPoint(point.position, point.penetration);
			}
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
		result.AddPoint((pointA + pointB) * 0.5f, minOverlap);
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

//drops all pointers to bodies/colliders (they dangle once the owning objects are destroyed)
void PhysicsWorld::Clear() {
	bodies.clear();
	colliders.clear();
	broadPhase.clear();
	contacts.clear();
	impulseCache.clear();
	triggerBuffer1.clear();
	triggerBuffer2.clear();
	triggerBufferSwitch = false;
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

		body->inContact = false;
	}
	if (triggerBufferSwitch) triggerBuffer2.clear();
	else triggerBuffer1.clear();

	BuildBroad();
	BroadPhase();
	DetectContacts(); //the only collision tests this step

	//warm start: begin from last step's pushes, so a stack starts already supported instead of from zero
	for (Contact& contact : contacts) {
		WarmStart(contact);
	}

	//iterate the velocity solve over the fixed contacts -> each pass carries corrections further through stacks/piles
	constexpr int solverIterations = 6;
	for (int i = 0; i < solverIterations; ++i) {
		for (Contact& contact : contacts) {
			SolveContact(contact);
		}
	}
	StoreImpulses();
	//fix leftover overlap once, after velocities are solved
	for (const Contact& contact : contacts) {
		CorrectPosition(contact);
	}

	//sleep: a body touching something that stays nearly still for a while has its leftover velocity zeroed
	//waiting (instead of zeroing at once) lets slow starts through, e.g. a crate just beginning to tip over
	constexpr float linearRestThreshold = 0.02f;  //m/s
	constexpr float angularRestThreshold = 0.02f; //rad/s
	constexpr float timeToRest = 0.5f;			  //seconds
	for (RigidBodyComponent* body : bodies) {
		if (body->isStatic) continue;
		bool nearlyStill = body->inContact
			&& glm::length2(body->velocity) < linearRestThreshold * linearRestThreshold
			&& glm::length2(body->angularVelocity) < angularRestThreshold * angularRestThreshold;
		body->restTimer = nearlyStill ? body->restTimer + dt : 0.0f;
		if (body->restTimer >= timeToRest) {
			body->velocity = glm::vec3(0.0f);
			body->angularVelocity = glm::vec3(0.0f);
		}
	}

	UpdateTrigger();
}
//velocity of a contact point on a body: linear + spin around the centre (spin skipped when it can't rotate)
static glm::vec3 PointVelocity(const RigidBodyComponent* body, bool rotates, const glm::vec3& r) {
	if (!body) return glm::vec3(0);
	if (!rotates) return body->GetVelocity();
	return body->GetVelocity() + glm::cross(body->GetAngularVelocity(), r);
}

//how much a unit impulse along `direction` changes the contact's relative velocity, as 1/x:
//inverse masses + rotational terms (an impulse partly turns into spin instead of movement)
static float Denominator(const Contact& contact, const glm::vec3& direction) {
	float denominator = contact.inverseMassA + contact.inverseMassB;
	if (contact.rotatesA) {
		glm::vec3 rdA = glm::cross(contact.rA, direction);
		denominator += glm::dot(direction, glm::cross(contact.inverseInertiaA * rdA, contact.rA));
	}
	if (contact.rotatesB) {
		glm::vec3 rdB = glm::cross(contact.rB, direction);
		denominator += glm::dot(direction, glm::cross(contact.inverseInertiaB * rdB, contact.rB));
	}
	return denominator;
}

//per touching pair: bodies, masses, inertia, friction, normal (shared by every contact point of the pair)
//false when neither body can move (static vs static)
bool PhysicsWorld::SetupBodies(const Result& result, Contact& contact) {
	RigidBodyComponent* ra = result.a->GetRigidBody();
	RigidBodyComponent* rb = result.b->GetRigidBody();

	if (ra && !ra->IsStatic()) {
		contact.inverseMassA = ra->GetInverseMass();
		contact.rotatesA = !ra->IsRotationFrozen();
		if (contact.rotatesA) contact.inverseInertiaA = ra->GetInverseInertiaWorld(); //respects partial locks
	}
	if (rb && !rb->IsStatic()) {
		contact.inverseMassB = rb->GetInverseMass();
		contact.rotatesB = !rb->IsRotationFrozen();
		if (contact.rotatesB) contact.inverseInertiaB = rb->GetInverseInertiaWorld();
	}
	if (contact.inverseMassA + contact.inverseMassB == 0.0f) return false;

	if (ra) ra->inContact = true;
	if (rb) rb->inContact = true;

	contact.a = ra;
	contact.b = rb;
	contact.normal = result.normal;
	//centres are only needed for rotation -> skipped (with their world matrix rebuild) when a body can't rotate
	if (contact.rotatesA) contact.centerA = ra->GetCenterMass();
	if (contact.rotatesB) contact.centerB = rb->GetCenterMass();

	//two directions along the surface, at right angles to the normal and each other
	//(build one from whichever axis isn't close to the normal, the second is the cross product)
	const glm::vec3& n = contact.normal;
	contact.tangent[0] = glm::normalize(glm::abs(n.x) > 0.57735f ? glm::vec3(n.y, -n.x, 0.0f) : glm::vec3(0.0f, n.z, -n.y));
	contact.tangent[1] = glm::cross(n, contact.tangent[0]);

	//geometric mean: if either surface is slippery the contact is slippery. no rigidbody -> default material
	constexpr float defaultFriction = 0.5f;
	float frictionA = ra ? ra->GetFriction() : defaultFriction;
	float frictionB = rb ? rb->GetFriction() : defaultFriction;
	contact.friction = std::sqrt(frictionA * frictionB);

	contact.positionWeight = 1.0f / static_cast<float>(result.pointCount);
	return true;
}

//per contact point: where it is, how deep, and everything that depends on the offset from the centres
void PhysicsWorld::SetupPoint(const ContactPoint& point, Contact& contact) {
	contact.position = point.position;
	contact.penetration = point.penetration;
	if (contact.rotatesA) contact.rA = point.position - contact.centerA;
	if (contact.rotatesB) contact.rB = point.position - contact.centerB;

	//only depend on geometry + masses -> compute once instead of every iteration
	contact.normalDenominator = Denominator(contact, contact.normal);
	contact.tangentDenominator[0] = Denominator(contact, contact.tangent[0]);
	contact.tangentDenominator[1] = Denominator(contact, contact.tangent[1]);

	//bounce target from the approach speed before solving: separate at restitution * approach speed
	//slow approaches don't bounce -> a resting bouncy ball doesn't twitch from gravity every step
	constexpr float bounceThreshold = 0.5f; //m/s
	float restitution = glm::max(contact.a ? contact.a->GetRestitution() : 0.0f, contact.b ? contact.b->GetRestitution() : 0.0f);
	glm::vec3 velocityA = PointVelocity(contact.a, contact.rotatesA, contact.rA);
	glm::vec3 velocityB = PointVelocity(contact.b, contact.rotatesB, contact.rB);
	float approachVelocity = glm::dot(contact.normal, velocityB - velocityA); //negative = moving together
	if (approachVelocity < -bounceThreshold) {
		contact.bounceVelocity = -restitution * approachVelocity;
	}
}

//one solver pass on one contact: velocities only, reads the bodies' current velocities
//accumulated impulses: each part keeps a running total, the TOTAL is clamped and only the change is applied
void PhysicsWorld::SolveContact(Contact& contact) {
	//normal: bring the separating speed up to the bounce target (0 without restitution)
	if (contact.normalDenominator > 1e-6f) {
		glm::vec3 relativeVelocity = PointVelocity(contact.b, contact.rotatesB, contact.rB) - PointVelocity(contact.a, contact.rotatesA, contact.rA);
		float velocityNormal = glm::dot(contact.normal, relativeVelocity);
		float delta = (contact.bounceVelocity - velocityNormal) / contact.normalDenominator;

		//the total push can't go below 0 (contacts never pull), but this pass may ease back an earlier over-push
		float oldTotal = contact.normalImpulse;
		contact.normalImpulse = glm::max(oldTotal + delta, 0.0f);
		ApplyImpulse(contact, (contact.normalImpulse - oldTotal) * contact.normal);
	}

	//friction: grip is limited by the TOTAL push (Coulomb: pressed harder -> more grip)
	float maxFriction = contact.friction * contact.normalImpulse;
	for (int i = 0; i < 2; ++i) {
		if (contact.tangentDenominator[i] <= 1e-6f) continue;
		//recomputed: the normal impulse (and the other tangent) just changed the velocities
		glm::vec3 relativeVelocity = PointVelocity(contact.b, contact.rotatesB, contact.rB) - PointVelocity(contact.a, contact.rotatesA, contact.rA);
		float delta = -glm::dot(relativeVelocity, contact.tangent[i]) / contact.tangentDenominator[i];

		float oldTotal = contact.tangentImpulse[i];
		contact.tangentImpulse[i] = glm::clamp(oldTotal + delta, -maxFriction, maxFriction);
		ApplyImpulse(contact, (contact.tangentImpulse[i] - oldTotal) * contact.tangent[i]);
	}
}

//impulse acts at the contact point: -impulse on a, +impulse on b (static bodies don't move, spin only if they can rotate)
void PhysicsWorld::ApplyImpulse(const Contact& contact, const glm::vec3& impulse) {
	RigidBodyComponent* ra = contact.a;
	RigidBodyComponent* rb = contact.b;
	if (ra && !ra->IsStatic()) {
		ra->velocity -= impulse * contact.inverseMassA;
		if (contact.rotatesA)
			ra->angularVelocity -= contact.inverseInertiaA * glm::cross(contact.rA, impulse);
	}
	if (rb && !rb->IsStatic()) {
		rb->velocity += impulse * contact.inverseMassB;
		if (contact.rotatesB)
			rb->angularVelocity += contact.inverseInertiaB * glm::cross(contact.rB, impulse);
	}
}

//pushes overlapping bodies apart, split by inverse mass
//single pass with the start-of-step penetration (replaces 6 passes x 25% that re-measured the overlap ~= 82%)
void PhysicsWorld::CorrectPosition(const Contact& contact) {
	//how aggressive to fix penetration
	constexpr float percent = 0.8f;
	//tollerance of penetration
	constexpr float slop = 0.01f;

	float totalInverseMass = contact.inverseMassA + contact.inverseMassB;
	float depth = glm::max(contact.penetration - slop, 0.0f);
	//each point does its share -> a flat box with 4 points isn't pushed out 4 times too far
	glm::vec3 correction = contact.normal * (depth / totalInverseMass) * percent * contact.positionWeight;

	if (contact.a) contact.a->Move(-correction * contact.inverseMassA);
	if (contact.b) contact.b->Move(correction * contact.inverseMassB);
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
			event.trigger = pair.a->IsTrigger() ? pair.a : pair.b;
			event.other = pair.a->IsTrigger() ? pair.b : pair.a;
			eventBus.Emit(event);
		}
	}

	for (auto& pair : *previousTrigger) {
		if (!currentTrigger->contains(pair)) {
			//leave
			TriggerExitEvent event;
			event.trigger = pair.a->IsTrigger() ? pair.a : pair.b;
			event.other = pair.a->IsTrigger() ? pair.b : pair.a;
			eventBus.Emit(event);
		}
	}
	triggerBufferSwitch = !triggerBufferSwitch;
}
//narrow phase on every broad phase pair -> trigger overlaps + solid contacts for the solver
void PhysicsWorld::DetectContacts() {
	contacts.clear();
	for (Pair& pair : broadPhase) {
		Result result;
		if (!Collision::Test(*pair.a, *pair.b, result)) continue;
		if (pair.a->IsTrigger() || pair.b->IsTrigger()) {
			if (triggerBufferSwitch) triggerBuffer2.insert(pair);
			else triggerBuffer1.insert(pair);
			continue;
		}
		Contact base;
		base.pair = pair;
		if (!SetupBodies(result, base)) continue;
		for (int i = 0; i < result.pointCount; ++i) {
			Contact contact = base;
			SetupPoint(result.points[i], contact);
			contacts.push_back(contact);
		}
	}
}

//starts a contact point from last step's totals and applies them
//matched to the nearest point the same two colliders had last step (points barely move in one step)
void PhysicsWorld::WarmStart(Contact& contact) {
	auto it = impulseCache.find(contact.pair);
	if (it == impulseCache.end()) return; //pair wasn't touching last step
	const CachedManifold& manifold = it->second;

	constexpr float matchDistance = 0.05f; //metres
	const CachedPoint* match = nullptr;
	float bestDistance2 = matchDistance * matchDistance;
	for (int i = 0; i < manifold.pointCount; ++i) {
		float distance2 = glm::length2(manifold.points[i].position - contact.position);
		if (distance2 < bestDistance2) {
			bestDistance2 = distance2;
			match = &manifold.points[i];
		}
	}
	if (!match) return; //new point this step

	//stored as the push on b -> flip if this step's pair is the other way round
	glm::vec3 friction = (manifold.a == contact.pair.a) ? match->friction : -match->friction;

	contact.normalImpulse = match->normal;
	float maxFriction = contact.friction * contact.normalImpulse;
	glm::vec3 impulse = contact.normalImpulse * contact.normal;
	for (int i = 0; i < 2; ++i) {
		//tangents are rebuilt every step -> project the stored friction onto this step's directions
		contact.tangentImpulse[i] = glm::clamp(glm::dot(friction, contact.tangent[i]), -maxFriction, maxFriction);
		impulse += contact.tangentImpulse[i] * contact.tangent[i];
	}
	ApplyImpulse(contact, impulse);
}

//keeps this step's totals for the next one, grouped by pair. only touching pairs are kept, so separated pairs drop out
void PhysicsWorld::StoreImpulses() {
	impulseCache.clear();
	for (const Contact& contact : contacts) {
		CachedManifold& manifold = impulseCache[contact.pair];
		manifold.a = contact.pair.a;
		if (manifold.pointCount >= Result::MaxPoints) continue;
		CachedPoint& cached = manifold.points[manifold.pointCount++];
		cached.position = contact.position;
		cached.normal = contact.normalImpulse;
		cached.friction = contact.tangentImpulse[0] * contact.tangent[0] + contact.tangentImpulse[1] * contact.tangent[1];
	}
}

void PhysicsWorld::Integrate(RigidBodyComponent* body, float dt) {
	TransformComponent* transform = body->owner->GetComponent<TransformComponent>();
	//linear
	glm::vec3 acceleration = body->accumulatedForce * body->inverseMass;
	body->velocity += acceleration * dt;
	body->velocity += body->pendingImpulse * body->inverseMass;
	transform->Translate(body->velocity * dt);

	//rotation fully locked -> skip all torque, inertia and spin work
	if (body->IsRotationFrozen()) {
		body->angularVelocity = glm::vec3(0.0f);
		return;
	}

	//angular
	glm::mat3 R = glm::mat3_cast(transform->GetQuat());
	glm::mat3 inverseInertiaWorld = R * body->GetInverseLocalInertiaTensor() * glm::transpose(R);

	glm::vec3 angularAcceleration = inverseInertiaWorld * body->accumulatedTorque;
	body->angularVelocity += angularAcceleration * dt;
	body->angularVelocity += inverseInertiaWorld * body->pendingAngularImpulse;

	body->ApplyConstraint();

	//time based, so it's the same at any step rate. 0.6/s ~= the old 0.995 per step at 120 steps/s
	constexpr float angularDamping = 0.6f;
	body->angularVelocity *= 1.0f / (1.0f + angularDamping * dt);
	float angularSpeed = glm::length(body->angularVelocity);
	if (angularSpeed < 1e-5f) {
		body->angularVelocity = glm::vec3(0.0f);
		return;
	}
	if (angularSpeed > 0) {
		glm::vec3 axis = body->angularVelocity / angularSpeed;
		float angle = angularSpeed * dt;

		glm::quat deltaRotation = glm::angleAxis(angle, axis);
		//angular velocity is world space -> pre-multiply (Rotate() post-multiplies, which spins about the local axis)
		transform->SetRotation(glm::normalize(deltaRotation * transform->GetLocalQuat()));

	}
}
