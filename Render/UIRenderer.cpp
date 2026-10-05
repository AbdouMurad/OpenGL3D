#include "UIRenderer.h"
#include "Core/AssetManager.h"

#include <cstddef>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_easy_font.h>

bool UIRenderer::init(int w, int h) {
	width = w;
	height = h;

	ShaderHandle shaderID = AssetManager::Get().LoadShader("assets/shaders/ui.vert", "assets/shaders/ui.frag");
	shaderPtr = &AssetManager::Get().GetShader(shaderID);
	vao.init();
	vao.Bind();
	vbo.Bind();

	std::vector<Vertex> empty;
	vbo.init(empty);

	vao.LinkAttrib(vbo, 0, 2, GL_FLOAT, sizeof(UIVertex), (void*)offsetof(UIVertex, position));
	vao.LinkAttrib(vbo, 1, 4, GL_FLOAT, sizeof(UIVertex), (void*)offsetof(UIVertex, color));
	vao.Unbind();

	return true;
}

void UIRenderer::Resize(int w, int h) {
	width = w;
	height = h;
}

void UIRenderer::DrawRect(float x, float y, float w, float h, const glm::vec4& color) {
	UIVertex topLeft = { { x, y }, color };
	UIVertex topRight = { { x + w, y }, color };
	UIVertex bottomRight = { { x + w, y + h }, color };
	UIVertex bottomLeft = { { x, y + h }, color };

	vertices.insert(vertices.end(), { topLeft, topRight, bottomRight, topLeft, bottomRight, bottomLeft });
}

void UIRenderer::DrawText(const std::string& text, float x, float y, float scale, const glm::vec4& color, bool shadow) {
	if (text.empty()) return;
	if (shadow) DrawText(text, x + scale, y + scale, scale, glm::vec4(0.0f, 0.0f, 0.0f, color.a * 0.75f), false);

	//stb_easy_font builds each glyph out of quads, 4 vertices each
	struct EasyFontVertex {
		float x, y, z;
		unsigned char color[4];
	};
	//it wants a mutable string, and averages ~270 bytes of vertices per character
	std::string mutableText = text;
	std::vector<EasyFontVertex> quads(text.size() * 48 + 16);
	int quadCount = stb_easy_font_print(0.0f, 0.0f, mutableText.data(), nullptr, quads.data(), static_cast<int>(quads.size() * sizeof(EasyFontVertex)));

	for (int i = 0; i < quadCount; ++i) {
		const EasyFontVertex* q = &quads[i * 4];
		//the font is drawn at the origin, scale it then move it into place
		float x0 = x + q[0].x * scale;
		float y0 = y + q[0].y * scale;
		float x1 = x + q[2].x * scale;
		float y1 = y + q[2].y * scale;
		DrawRect(x0, y0, x1 - x0, y1 - y0, color);
	}
}

void UIRenderer::DrawTextCentered(const std::string& text, float centerX, float y, float scale, const glm::vec4& color, bool shadow) {
	DrawText(text, centerX - TextWidth(text, scale) * 0.5f, y, scale, color, shadow);
}

float UIRenderer::TextWidth(const std::string& text, float scale) const {
	std::string mutableText = text;
	return stb_easy_font_width(mutableText.data()) * scale;
}

void UIRenderer::Render() {
	if (vertices.empty() || !shaderPtr) return;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shaderPtr->Activate();
	//y down so (0,0) is the top left corner
	shaderPtr->setMat4("u_projection", glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f));

	vao.Bind();
	vbo.SetType(vertices.data(), vertices.size() * sizeof(UIVertex), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
	vao.Unbind();

	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	vertices.clear();
}
