#include "GrassField.h"
#include "GPU/Texture.h"

#include <cmath>
#include <cstddef>
#include <random>

GrassField::GrassField(const GrassSettings& settings) {
	groundHeight = settings.groundHeight;
	BuildBlade();
	BuildChunks(settings);
	BuildTrailMap(settings);
}

GrassField::~GrassField() {
	//the game is destroyed after the window closes: no context then, and GL calls would go nowhere
	if (!glfwGetCurrentContext()) return;
	for (GrassChunk& chunk : chunks) {
		chunk.instances.Delete();
		chunk.vao.Delete();
	}
	bladeVertices.Delete();
	bladeIndices.Delete();
	glDeleteTextures(1, &trailTexture);
}

//a flat, tapering strip 1 tall and 1 wide at the root, in x/y. uv.y = how far up the blade (0 root .. 1 tip)
//the shader scales it per instance and bends the top, so the strip has a few rows to bend smoothly
void GrassField::BuildBlade() {
	const float rowHeights[4] = { 0.0f, 0.3f, 0.6f, 0.85f };
	std::vector<Vertex> vertices;
	for (float h : rowHeights) {
		float halfWidth = 0.5f * (1.0f - std::pow(h, 1.5f)); //narrows towards the top
		for (float side : { -1.0f, 1.0f }) {
			Vertex v;
			v.position = glm::vec3(side * halfWidth, h, 0.0f);
			v.normal = glm::vec3(0, 0, 1);
			v.texUV = glm::vec2(side * 0.5f + 0.5f, h);
			vertices.push_back(v);
		}
	}
	Vertex tip;
	tip.position = glm::vec3(0.0f, 1.0f, 0.0f);
	tip.normal = glm::vec3(0, 0, 1);
	tip.texUV = glm::vec2(0.5f, 1.0f);
	vertices.push_back(tip);

	//two triangles between each pair of rows, one for the tip (both sides are drawn, so winding doesn't matter)
	std::vector<GLuint> indices;
	for (GLuint row = 0; row < 3; ++row) {
		GLuint a = row * 2;
		indices.insert(indices.end(), { a, a + 1, a + 3, a, a + 3, a + 2 });
	}
	indices.insert(indices.end(), { 6, 7, 8 });
	indexCount = static_cast<GLsizei>(indices.size());

	bladeVertices.init(vertices);
	//filled through GL_ARRAY_BUFFER: binding GL_ELEMENT_ARRAY_BUFFER needs a VAO bound in core profile.
	//buffers have no fixed type, each chunk's VAO binds it as its index buffer below
	glGenBuffers(1, &bladeIndices.ID);
	glBindBuffer(GL_ARRAY_BUFFER, bladeIndices.ID);
	glBufferData(GL_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//cheap smooth "noise" from a few sines: patches of taller and shorter grass across the field
static float Patchiness(float x, float z) {
	float n = std::sin(x * 0.11f + 1.7f) * std::sin(z * 0.13f - 0.4f)
			+ 0.5f * std::sin(x * 0.31f - z * 0.27f + 2.1f);
	return 0.85f + 0.3f * n; //~0.4 .. 1.3
}

void GrassField::BuildChunks(const GrassSettings& settings) {
	std::mt19937 rng(settings.seed);
	std::uniform_real_distribution<float> unit(0.0f, 1.0f);

	glm::vec2 fieldMin = settings.center - settings.halfSize;
	glm::vec2 fieldMax = settings.center + settings.halfSize;
	int chunksX = std::max(1, static_cast<int>(std::ceil((fieldMax.x - fieldMin.x) / settings.chunkSize)));
	int chunksZ = std::max(1, static_cast<int>(std::ceil((fieldMax.y - fieldMin.y) / settings.chunkSize)));
	chunks.resize(static_cast<size_t>(chunksX) * chunksZ);

	for (int cz = 0; cz < chunksZ; ++cz) {
		for (int cx = 0; cx < chunksX; ++cx) {
			GrassChunk& chunk = chunks[static_cast<size_t>(cz) * chunksX + cx];
			glm::vec2 min = fieldMin + glm::vec2(cx, cz) * settings.chunkSize;
			glm::vec2 max = glm::min(min + glm::vec2(settings.chunkSize), fieldMax);
			glm::vec2 size = max - min;

			//random order inside the chunk -> drawing only the first N blades thins it out evenly (distance LOD)
			int count = static_cast<int>(size.x * size.y * settings.density);
			std::vector<GrassInstance> instances(count);
			for (GrassInstance& blade : instances) {
				float x = min.x + unit(rng) * size.x;
				float z = min.y + unit(rng) * size.y;
				float height = glm::mix(settings.minHeight, settings.maxHeight, unit(rng)) * Patchiness(x, z);
				float width = settings.bladeWidth * glm::mix(0.75f, 1.3f, unit(rng));
				float lean = unit(rng) * 0.35f;
				blade.rootAndYaw = glm::vec4(x, settings.groundHeight, z, unit(rng) * 6.2831853f);
				blade.params = glm::vec4(height, width, lean, unit(rng));
			}
			chunk.count = count;
			bladeCount += count;

			//bending can push a tip ~a blade's height sideways, so the box gets that much room
			float reach = settings.maxHeight * 1.3f;
			chunk.bounds.Expand(glm::vec3(min.x - reach, settings.groundHeight, min.y - reach));
			chunk.bounds.Expand(glm::vec3(max.x + reach, settings.groundHeight + reach, max.y + reach));

			chunk.vao.init();
			chunk.vao.Bind();
			//per vertex: the shared blade mesh (same layout as Vertex, like every other mesh)
			chunk.vao.LinkAttrib(bladeVertices, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
			chunk.vao.LinkAttrib(bladeVertices, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normal));
			chunk.vao.LinkAttrib(bladeVertices, 2, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, texUV));
			bladeIndices.Bind(); //stored in the VAO

			//per instance: divisor 1 = advance once per blade instead of once per vertex
			glGenBuffers(1, &chunk.instances.ID);
			chunk.instances.SetType(instances.data(), instances.size() * sizeof(GrassInstance), GL_STATIC_DRAW);
			chunk.vao.LinkAttrib(chunk.instances, 4, 4, GL_FLOAT, sizeof(GrassInstance), (void*)offsetof(GrassInstance, rootAndYaw));
			chunk.vao.LinkAttrib(chunk.instances, 5, 4, GL_FLOAT, sizeof(GrassInstance), (void*)offsetof(GrassInstance, params));
			glVertexAttribDivisor(4, 1);
			glVertexAttribDivisor(5, 1);
			chunk.vao.Unbind();
		}
	}
}

//-------------------Trail-----------------
//texels start "pressed long ago with no strength" -> nothing flattened
void GrassField::BuildTrailMap(const GrassSettings& settings) {
	trailMin = settings.center - settings.halfSize;
	trailSize = settings.halfSize * 2.0f;
	trailWidth = std::max(1, static_cast<int>(std::ceil(trailSize.x / settings.trailTexelSize)));
	trailHeight = std::max(1, static_cast<int>(std::ceil(trailSize.y / settings.trailTexelSize)));
	trailData.assign(static_cast<size_t>(trailWidth) * trailHeight, glm::vec4(0.0f, 0.0f, -1000.0f, 0.0f));

	glGenTextures(1, &trailTexture);
	glActiveTexture(GL_TEXTURE0 + GRASS_TRAIL_SLOT);
	glBindTexture(GL_TEXTURE_2D, trailTexture);
	//32 bit float: the press time needs full precision (half floats are ~0.5 s apart after a few minutes)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, trailWidth, trailHeight, 0, GL_RGBA, GL_FLOAT, trailData.data());
	//nearest: blending a press time with "never pressed" would give a meaningless time in between
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//stays flat for the first 35% of the recovery time, then eases back up
float GrassField::Remaining(float age) const {
	float t = glm::clamp(age / trailRecoverTime, 0.0f, 1.0f);
	return 1.0f - glm::smoothstep(0.35f, 1.0f, t);
}

void GrassField::Stamp(glm::vec2 center, glm::vec2 direction, float radius, float time) {
	glm::vec2 texel = trailSize / glm::vec2(trailWidth, trailHeight);
	//texels the circle covers, clamped to the map
	glm::ivec2 first = glm::ivec2(glm::floor((center - radius - trailMin) / texel));
	glm::ivec2 last = glm::ivec2(glm::floor((center + radius - trailMin) / texel));
	first = glm::max(first, glm::ivec2(0));
	last = glm::min(last, glm::ivec2(trailWidth - 1, trailHeight - 1));
	if (first.x > last.x || first.y > last.y) return; //entirely off the field

	glm::vec2 across(-direction.y, direction.x); //sideways to the path

	for (int z = first.y; z <= last.y; ++z) {
		for (int x = first.x; x <= last.x; ++x) {
			glm::vec2 offset = trailMin + (glm::vec2(x, z) + 0.5f) * texel - center;
			float distance = glm::length(offset);
			if (distance > radius) continue;

			//full press in the middle, soft edge
			float strength = 1.0f - glm::smoothstep(radius * 0.55f, radius, distance);
			glm::vec4& t = trailData[static_cast<size_t>(z) * trailWidth + x];
			//a deeper print that's still there wins (e.g. the middle of the path when the edge of the next step overlaps it)
			if (strength < t.a * Remaining(time - t.b)) continue;

			//along the walking direction, tipped outward on either side of the path's centre line
			float side = glm::dot(offset, across) / radius; //-1 .. 1
			glm::vec2 lay = glm::normalize(direction + across * side * 0.7f);
			t = glm::vec4(lay.x, lay.y, time, strength);
		}
	}

	//upload only the changed rectangle. ROW_LENGTH: rows in trailData are the full map width
	glActiveTexture(GL_TEXTURE0 + GRASS_TRAIL_SLOT);
	glBindTexture(GL_TEXTURE_2D, trailTexture);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, trailWidth);
	glTexSubImage2D(GL_TEXTURE_2D, 0, first.x, first.y, last.x - first.x + 1, last.y - first.y + 1, GL_RGBA, GL_FLOAT,
		&trailData[static_cast<size_t>(first.y) * trailWidth + first.x]);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
