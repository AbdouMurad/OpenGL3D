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
	
	float radiusSum = sphereA->radius + sphereB->radius;
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
		r.AddContact(pointA + r.normal * sphereA->radius, penetration);
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
	const float radius = sphereB->radius;
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
			result.AddContact(glm::vec3(matB[3]) - result.normal * radius, penetration);
		}
		else {
			result.normal = diff / distance;
			result.AddContact(closest, radius - distance);
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
		float distance = glm::dot(faceNormal, point) - referencePlane;

		if (distance <= epsilon) {
			//each point keeps its own depth, a tilted box is deeper at one end
			manifold.push_back({ point - faceNormal * distance, -distance });
		}
	}

	return manifold;
}
// clipping can give up to 8 points, keep the 4 that best describe the contact area:
// the deepest, then the points that make the supporting shape as wide as possible
std::vector<ContactPoint> Collision::ReduceManifold(const std::vector<ContactPoint>& points, const glm::vec3& normal) {
	if (points.size() <= Result::MaxContacts) return points;

	size_t deepest = 0;
	for (size_t i = 1; i < points.size(); ++i) {
		if (points[i].penetration > points[deepest].penetration) deepest = i;
	}
	const glm::vec3& p0 = points[deepest].position;

	size_t farthest = 0;
	float bestDistance = -1.0f;
	for (size_t i = 0; i < points.size(); ++i) {
		float distance = glm::length2(points[i].position - p0);
		if (distance > bestDistance) {
			bestDistance = distance;
			farthest = i;
		}
	}
	const glm::vec3& p1 = points[farthest].position;

	//signed area of triangle p0,p1,p: sign says which side of the p0-p1 line p is on
	auto signedArea = [&](const glm::vec3& p) {
		return glm::dot(glm::cross(p1 - p0, p - p0), normal);
	};

	size_t third = 0;
	float bestArea = -1.0f;
	for (size_t i = 0; i < points.size(); ++i) {
		float area = std::abs(signedArea(points[i].position));
		if (area > bestArea) {
			bestArea = area;
			third = i;
		}
	}

	//fourth point: biggest triangle on the other side of p0-p1 from the third
	float side = signedArea(points[third].position) >= 0.0f ? -1.0f : 1.0f;
	size_t fourth = points.size();
	bestArea = 0.0f;
	for (size_t i = 0; i < points.size(); ++i) {
		float area = side * signedArea(points[i].position);
		if (area > bestArea) {
			bestArea = area;
			fourth = i;
		}
	}

	std::vector<ContactPoint> reduced = { points[deepest], points[farthest], points[third] };
	if (fourth < points.size()) reduced.push_back(points[fourth]);
	return reduced;
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

	float bestFaceOverlap = FLT_MAX;
	int bestFaceIndex = -1;
	float bestEdgeOverlap = FLT_MAX;
	int bestEdgeIndex = -1;
	for (int i = 0; i < index; ++i) {
		glm::vec3 axis = axes[i].axis;

		float distance = abs(glm::dot(delta, axis));

		float radiusA = ProjectRadius(boxA, axis);
		float radiusB = ProjectRadius(boxB, axis);

		float overlap = radiusA + radiusB - distance;
		if (overlap < 0) return false;

		if (i < 6 && overlap < bestFaceOverlap) {
			bestFaceOverlap = overlap;
			bestFaceIndex = i;
		}
		else if (i >= 6 && overlap < bestEdgeOverlap) {
			bestEdgeOverlap = overlap;
			bestEdgeIndex = i;
		}
	}
	//a face axis gives a full manifold, an edge axis only one point. When they're almost tied
	//(a box resting flat has many) picking by tiny float differences makes contacts flicker
	//between the two from frame to frame, so only use the edge axis when it's clearly better
	constexpr float relativeTolerance = 0.95f;
	constexpr float absoluteTolerance = 0.01f;
	int bestAxisIndex = bestFaceIndex;
	float minOverlap = bestFaceOverlap;
	if (bestEdgeIndex >= 0 && bestEdgeOverlap < relativeTolerance * bestFaceOverlap - absoluteTolerance) {
		bestAxisIndex = bestEdgeIndex;
		minOverlap = bestEdgeOverlap;
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
			result.AddContact((FurthestPoint(boxA, result.normal) + FurthestPoint(boxB, -result.normal)) * 0.5f, minOverlap);
		}
		else {
			//every point gets resolved, so a box resting on a face is held up at all corners
			for (const ContactPoint& contact : ReduceManifold(contacts, result.normal)) {
				result.AddContact(contact.position, contact.penetration);
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
		result.AddContact((pointA + pointB) * 0.5f, minOverlap);
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
	//1. forces -> velocities
	for (RigidBodyComponent* body : bodies) {
		if (!body->isStatic) IntegrateVelocity(body, dt);

		body->accumulatedForce = glm::vec3(0);
		body->accumulatedTorque = glm::vec3(0);

		body->pendingImpulse = glm::vec3(0);
		body->pendingAngularImpulse = glm::vec3(0);
	}
	if (triggerBufferSwitch) triggerBuffer2.clear();
	else triggerBuffer1.clear();

	//2. find contacts at the current positions
	BuildBroad();
	BroadPhase();
	NarrowPhase();

	//3. fix velocities so nothing moves into anything else. Done before moving, so a resting
	//object's gravity gets cancelled instead of sinking it into the floor and pushing it back out
	solver.Prepare(contacts, dt);
	for (int i = 0; i < solverIterations; ++i) {
		solver.SolveVelocities();
	}

	//4. velocities -> positions
	for (RigidBodyComponent* body : bodies) {
		if (!body->isStatic) IntegratePosition(body, dt);
	}
	UpdateTrigger();
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
void PhysicsWorld::NarrowPhase() {
	contacts.clear();
	std::unordered_set<Pair, PairHash>& currentTrigger = triggerBufferSwitch ? triggerBuffer2 : triggerBuffer1;
	for (Pair& pair : broadPhase) {
		Result result;
		if (!Collision::Test(*pair.a, *pair.b, result)) continue;
		if (pair.a->isTrigger || pair.b->isTrigger) {
			currentTrigger.insert(pair);
			continue;
		}
		contacts.push_back(result);
	}
}

void PhysicsWorld::IntegrateVelocity(RigidBodyComponent* body, float dt) {
	glm::vec3 force = body->accumulatedForce;
	if (body->useGravity) force += gravity * body->mass;

	//linear
	body->velocity += force * body->inverseMass * dt;
	body->velocity += body->pendingImpulse * body->inverseMass;

	//angular
	glm::mat3 inverseInertiaWorld = body->GetInverseInertiaWorld();
	body->angularVelocity += inverseInertiaWorld * body->accumulatedTorque * dt;
	body->angularVelocity += inverseInertiaWorld * body->pendingAngularImpulse;

	//dividing instead of multiplying by a constant keeps damping the same at any step size
	body->velocity *= 1.0f / (1.0f + dt * body->linearDamping);
	body->angularVelocity *= 1.0f / (1.0f + dt * body->angularDamping);

	body->ApplyConstraint();
}

void PhysicsWorld::IntegratePosition(RigidBodyComponent* body, float dt) {
	TransformComponent* transform = body->owner->GetComponent<TransformComponent>();
	transform->Translate(body->velocity * dt);

	float angularSpeed = glm::length(body->angularVelocity);
	if (angularSpeed < 1e-5f) {
		body->angularVelocity = glm::vec3(0.0f);
		return;
	}
	glm::vec3 axis = body->angularVelocity / angularSpeed;
	glm::quat deltaRotation = glm::angleAxis(angularSpeed * dt, axis);

	//angular velocity is in world space, so the delta goes on the left (world frame).
	//TransformComponent::Rotate multiplies on the right, which rotates around local axes instead.
	//uses the local quat because physics bodies are assumed to have no parent
	transform->SetRotation(glm::normalize(deltaRotation * transform->GetLocalQuat()));
}

void PhysicsWorld::DebugRender(DebugRenderer& renderer) {
	//contact points as small crosses, with a line along the contact normal
	constexpr float crossSize = 0.05f;
	constexpr float normalLength = 0.3f;
	for (const Result& manifold : contacts) {
		for (int i = 0; i < manifold.contactCount; ++i) {
			glm::vec3 p = manifold.contacts[i].position;
			renderer.DrawLine(p - glm::vec3(crossSize, 0, 0), p + glm::vec3(crossSize, 0, 0));
			renderer.DrawLine(p - glm::vec3(0, crossSize, 0), p + glm::vec3(0, crossSize, 0));
			renderer.DrawLine(p - glm::vec3(0, 0, crossSize), p + glm::vec3(0, 0, crossSize));
			renderer.DrawLine(p, p + manifold.normal * normalLength);
		}
	}
}

//slab test in the box's local frame: the ray is inside the box between the latest entry and the earliest exit
static bool RayBox(const glm::vec3& origin, const glm::vec3& direction, ColliderComponent& c, float& distance, glm::vec3& normal) {
	glm::mat4 mat = c.GetMatrix();
	glm::vec3 center = mat[3];
	glm::vec3 axes[3] = {
		glm::normalize(glm::vec3(mat[0])),
		glm::normalize(glm::vec3(mat[1])),
		glm::normalize(glm::vec3(mat[2]))
	};
	glm::vec3 halfExtent = static_cast<Box*>(c.shape.get())->halfExtent;

	float tEnter = -FLT_MAX;
	float tExit = FLT_MAX;
	glm::vec3 enterNormal = -direction;
	for (int i = 0; i < 3; ++i) {
		float o = glm::dot(origin - center, axes[i]);
		float d = glm::dot(direction, axes[i]);
		if (std::abs(d) < 1e-6f) {
			//parallel to this slab, either always inside it or never
			if (std::abs(o) > halfExtent[i]) return false;
			continue;
		}
		float t1 = (-halfExtent[i] - o) / d;
		float t2 = (halfExtent[i] - o) / d;
		if (t1 > t2) std::swap(t1, t2);
		if (t1 > tEnter) {
			tEnter = t1;
			enterNormal = d > 0.0f ? -axes[i] : axes[i];
		}
		tExit = std::min(tExit, t2);
		if (tEnter > tExit) return false;
	}
	if (tExit < 0.0f) return false;

	//origin inside the box counts as a hit right away
	distance = std::max(tEnter, 0.0f);
	normal = tEnter >= 0.0f ? enterNormal : -direction;
	return true;
}

static bool RaySphere(const glm::vec3& origin, const glm::vec3& direction, ColliderComponent& c, float& distance, glm::vec3& normal) {
	glm::vec3 center = c.GetMatrix()[3];
	float radius = static_cast<Sphere*>(c.shape.get())->radius;

	glm::vec3 m = origin - center;
	float b = glm::dot(m, direction);
	float k = glm::dot(m, m) - radius * radius;
	//outside and pointing away
	if (k > 0.0f && b > 0.0f) return false;
	float discriminant = b * b - k;
	if (discriminant < 0.0f) return false;

	distance = std::max(-b - std::sqrt(discriminant), 0.0f);
	glm::vec3 point = origin + direction * distance;
	normal = glm::length2(point - center) > 1e-8f ? glm::normalize(point - center) : -direction;
	return true;
}

bool PhysicsWorld::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit& hit, const GameObject* ignore) const {
	bool found = false;
	hit.distance = maxDistance;
	for (ColliderComponent* collider : colliders) {
		if (collider->isTrigger || collider->getOwner() == ignore) continue;

		float distance = 0.0f;
		glm::vec3 normal(0.0f);
		bool intersects = false;
		switch (collider->shape->GetType()) {
		case SHAPE::Box:
			intersects = RayBox(origin, direction, *collider, distance, normal);
			break;
		case SHAPE::Sphere:
			intersects = RaySphere(origin, direction, *collider, distance, normal);
			break;
		default:
			break;
		}
		if (!intersects || distance > hit.distance) continue;

		found = true;
		hit.collider = collider;
		hit.distance = distance;
		hit.normal = normal;
	}
	if (found) hit.point = origin + direction * hit.distance;
	return found;
}

void PhysicsWorld::RemoveObject(GameObject& object) {
	//after a step the switch has already flipped, so the pairs found last step are in the "previous" buffer
	std::unordered_set<Pair, PairHash>& latest = triggerBufferSwitch ? triggerBuffer1 : triggerBuffer2;
	std::unordered_set<Pair, PairHash>& stale = triggerBufferSwitch ? triggerBuffer2 : triggerBuffer1;

	auto involves = [&object](const Pair& pair) {
		return pair.a->getOwner() == &object || pair.b->getOwner() == &object;
	};
	for (auto it = latest.begin(); it != latest.end();) {
		if (!involves(*it)) {
			++it;
			continue;
		}
		TriggerExitEvent event;
		event.trigger = it->a->isTrigger ? it->a : it->b;
		event.other = it->a->isTrigger ? it->b : it->a;
		it = latest.erase(it);
		eventBus.Emit(event);
	}
	std::erase_if(stale, involves);

	std::erase_if(colliders, [&object](ColliderComponent* c) { return c->getOwner() == &object; });
	std::erase_if(bodies, [&object](RigidBodyComponent* b) { return b->getOwner() == &object; });
	std::erase_if(broadPhase, involves);
	std::erase_if(contacts, [&object](const Result& r) { return r.a->getOwner() == &object || r.b->getOwner() == &object; });
}
