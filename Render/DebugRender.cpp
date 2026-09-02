#include "DebugRender.h"

bool DebugRenderer::Init() {
	return true;
}
bool DebugRenderer::IsDebug() const {
	return debug;
}
void DebugRenderer::ToggleDebug() {
	debug = !debug;
}

void DebugRenderer::DrawLine(const glm::vec3& start, const glm::vec3& end) {
	vertices.push_back(start);
	vertices.push_back(end);
}
void DebugRenderer::DrawBox(const glm::vec3& center, const glm::vec3 axes[3], const glm::vec3& halfExtent) {
	glm::vec3 x = axes[0] * halfExtent.x;
	glm::vec3 y = axes[1] * halfExtent.y;
	glm::vec3 z = axes[2] * halfExtent.z;

	glm::vec3 corners[8] = {
		center - x - y - z,
		center + x - y - z,
		center + x + y - z,
		center - x + y - z,

		center - x - y + z,
		center + x - y + z,
		center + x + y + z,
		center - x + y + z
	};
	DrawLine(corners[0], corners[1]);
	DrawLine(corners[1], corners[2]);
	DrawLine(corners[2], corners[3]);
	DrawLine(corners[3], corners[0]);

	DrawLine(corners[4], corners[5]);
	DrawLine(corners[5], corners[6]);
	DrawLine(corners[6], corners[7]);
	DrawLine(corners[7], corners[4]);

	DrawLine(corners[0], corners[4]);
	DrawLine(corners[1], corners[5]);
	DrawLine(corners[2], corners[6]);
	DrawLine(corners[3], corners[7]);
}

void DebugRenderer::DrawSphere(const glm::vec3& center, float radius) {
	constexpr int segments = 24;
	for (int i = 0; i < segments; ++i) {
		float a0 = glm::two_pi<float>() * i / segments;
		float a1 = glm::two_pi<float>() * (i + 1) / segments;

		DrawLine(
			center + glm::vec3(cos(a0) * radius, sin(a0) * radius, 0.0f),
			center + glm::vec3(cos(a1) * radius, sin(a1) * radius, 0.0f)
		);
		DrawLine(
			center + glm::vec3(cos(a0) * radius, 0.0f, sin(a0) * radius),
			center + glm::vec3(cos(a1) * radius, 0.0f, sin(a1) * radius)
		);
		DrawLine(
			center + glm::vec3(0.0f, cos(a0) * radius, sin(a0) * radius),
			center + glm::vec3(0.0f, cos(a1) * radius, sin(a1) * radius)
		);
	}
}