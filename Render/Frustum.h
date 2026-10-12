#pragma once

#include <glm/glm.hpp>
#include <cfloat>

//axis aligned box, starts empty (min > max) so the first Expand() sets it
struct BoundingBox {
	glm::vec3 min = glm::vec3(FLT_MAX);
	glm::vec3 max = glm::vec3(-FLT_MAX);

	bool IsValid() const;
	void Expand(const glm::vec3& point);
	void Expand(const BoundingBox& box);

	//box around this box after moving it by matrix (stays axis aligned -> a bit bigger when rotated)
	BoundingBox Transformed(const glm::mat4& matrix) const;
};

//the 6 planes of the camera's view volume, in world space
class Frustum {
public:
	//from projection * view (Gribb-Hartmann: each plane is a sum/difference of the matrix's rows)
	void Extract(const glm::mat4& viewProjection);

	//false only when the box is fully outside one plane -> can keep a few invisible boxes, never drops a visible one
	bool IsVisible(const BoundingBox& box) const;

private:
	//xyz = normal pointing into the frustum, w = distance. inside when dot(normal, p) + w >= 0
	glm::vec4 planes[6];
};
