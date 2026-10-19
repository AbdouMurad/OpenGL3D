#pragma once

#include <vector>

#include "GPU/Buffer.h"
#include "Frustum.h"

//where and how grass grows. everything is generated from the seed, so the same settings give the same field
struct GrassSettings {
	glm::vec2 center = glm::vec2(0.0f);	 //middle of the field (x, z)
	glm::vec2 halfSize = glm::vec2(25.0f); //reaches this far from the centre on x and z
	float groundHeight = 0.0f;			 //y the blades grow from
	float density = 12.0f;				 //blades per square metre
	float chunkSize = 8.0f;				 //field is split into squares this wide, each culled and lit on its own
	float minHeight = 0.35f;
	float maxHeight = 0.8f;
	float bladeWidth = 0.07f;			 //at the root, tapers to a point
	float trailTexelSize = 0.2f;		 //metres per texel of the trail map (smaller = finer footprints, more memory)
	unsigned int seed = 1337;
};

//one blade, as the vertex shader reads it (two vec4 attributes per instance)
struct GrassInstance {
	glm::vec4 rootAndYaw; //xyz = root position, w = rotation around y (radians)
	glm::vec4 params;	  //x = height, y = width, z = lean, w = random 0..1 (colour + sway variation)
};

//a square of the field: drawn in one instanced call
struct GrassChunk {
	VAO vao;		  //blade mesh + this chunk's instances
	VBO instances;
	int count = 0;
	BoundingBox bounds; //includes the tallest blade, plus room for bending
};

//thousands of grass blades drawn with instancing: one small blade mesh, drawn once per instance with its own
//position/size/rotation. the renderer draws it (Renderer::DrawGrass); wind and bending happen in grass.vert
class GrassField {
public:
	explicit GrassField(const GrassSettings& settings);
	~GrassField();
	//owns GPU buffers -> no copies (two objects would delete the same buffers)
	GrassField(const GrassField&) = delete;
	GrassField& operator=(const GrassField&) = delete;

	const std::vector<GrassChunk>& GetChunks() const { return chunks; }
	GLsizei GetIndexCount() const { return indexCount; }
	int GetBladeCount() const { return bladeCount; }

	glm::vec3 rootColor = glm::vec3(0.02f, 0.06f, 0.015f);
	glm::vec3 tipColor = glm::vec3(0.22f, 0.55f, 0.08f);
	glm::vec2 windDirection = glm::normalize(glm::vec2(1.0f, 0.35f));
	float windStrength = 1.0f;

	//---- trail: grass pressed flat by something walking through, slowly standing back up ----
	//the trail map is a top-down texture over the field, one texel per trailTexelSize metres:
	//  rg = direction the grass was pressed (x, z), b = when (seconds, same clock as the shader), a = how hard 0..1
	//the shader works out the recovery from b, so the map only changes where something steps
	float trailRecoverTime = 8.0f; //seconds from last pressed until standing fully again

	//presses the grass in a circle around center, laid down along direction (normalized, x/z)
	//blades either side of the path lean a little outward, like a parted path
	void Stamp(glm::vec2 center, glm::vec2 direction, float radius, float time);
	//how much of a press made `age` seconds ago is left: 1 = still flat .. 0 = standing again (matches grass.vert)
	float Remaining(float age) const;

	GLuint GetTrailTexture() const { return trailTexture; }
	//xy = world x/z of the map's corner, zw = 1 / size in metres: uv = (worldXZ - xy) * zw
	glm::vec4 GetTrailRect() const { return glm::vec4(trailMin, 1.0f / trailSize); }
	float GetGroundHeight() const { return groundHeight; }

private:
	VBO bladeVertices;
	EBO bladeIndices;
	GLsizei indexCount = 0;
	int bladeCount = 0;
	std::vector<GrassChunk> chunks;
	float groundHeight = 0.0f;

	GLuint trailTexture = 0;
	int trailWidth = 0;
	int trailHeight = 0;
	glm::vec2 trailMin = glm::vec2(0.0f);
	glm::vec2 trailSize = glm::vec2(1.0f);
	std::vector<glm::vec4> trailData; //CPU copy: Stamp() reads it to not overwrite deeper prints

	void BuildBlade();
	void BuildChunks(const GrassSettings& settings);
	void BuildTrailMap(const GrassSettings& settings);
};
