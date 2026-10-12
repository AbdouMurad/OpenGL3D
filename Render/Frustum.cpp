#include "Frustum.h"

//-------------------Bounding Box-----------------
bool BoundingBox::IsValid() const {
	return min.x <= max.x && min.y <= max.y && min.z <= max.z;
}
void BoundingBox::Expand(const glm::vec3& point) {
	min = glm::min(min, point);
	max = glm::max(max, point);
}
void BoundingBox::Expand(const BoundingBox& box) {
	if (!box.IsValid()) return;
	Expand(box.min);
	Expand(box.max);
}
BoundingBox BoundingBox::Transformed(const glm::mat4& matrix) const {
	if (!IsValid()) return *this;

	//Arvo's method: move the centre, and the half size by abs() of the rotation/scale part
	glm::vec3 center = (min + max) * 0.5f;
	glm::vec3 halfSize = (max - min) * 0.5f;

	glm::mat3 absMatrix(matrix);
	for (int i = 0; i < 3; ++i) {
		absMatrix[i] = glm::abs(absMatrix[i]);
	}

	glm::vec3 newCenter = glm::vec3(matrix * glm::vec4(center, 1.0f));
	glm::vec3 newHalfSize = absMatrix * halfSize;

	BoundingBox result;
	result.min = newCenter - newHalfSize;
	result.max = newCenter + newHalfSize;
	return result;
}

//-------------------Frustum-----------------
void Frustum::Extract(const glm::mat4& m) {
	//glm is column major -> row i is (m[0][i], m[1][i], m[2][i], m[3][i])
	glm::vec4 row0(m[0][0], m[1][0], m[2][0], m[3][0]);
	glm::vec4 row1(m[0][1], m[1][1], m[2][1], m[3][1]);
	glm::vec4 row2(m[0][2], m[1][2], m[2][2], m[3][2]);
	glm::vec4 row3(m[0][3], m[1][3], m[2][3], m[3][3]);

	//clip space visible when -w <= x,y,z <= w, e.g. x + w >= 0 -> (row3 + row0) . p >= 0
	planes[0] = row3 + row0; //left
	planes[1] = row3 - row0; //right
	planes[2] = row3 + row1; //bottom
	planes[3] = row3 - row1; //top
	planes[4] = row3 + row2; //near
	planes[5] = row3 - row2; //far

	//normalize so w is a real distance (needed to compare against the box's projected radius)
	for (glm::vec4& plane : planes) {
		plane /= glm::length(glm::vec3(plane));
	}
}

bool Frustum::IsVisible(const BoundingBox& box) const {
	glm::vec3 center = (box.min + box.max) * 0.5f;
	glm::vec3 halfSize = (box.max - box.min) * 0.5f;

	for (const glm::vec4& plane : planes) {
		glm::vec3 normal(plane);
		//how far the box reaches towards this plane's normal
		float radius = glm::dot(glm::abs(normal), halfSize);
		float distance = glm::dot(normal, center) + plane.w;
		if (distance < -radius) return false; //whole box is behind this plane
	}
	return true;
}
