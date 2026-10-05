#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "GPU/Buffer.h"
#include "GPU/shaderClass.h"

// Screen space drawing for HUDs and menus. Coordinates are pixels with (0,0) at the top left.
// Everything queued during a frame is drawn on top of the scene by one Render call.
// Colors are sRGB, the same values a color picker gives.
class UIRenderer {
public:
	bool init(int width, int height);
	void Resize(int width, int height);
	void Render();

	int Width() const { return width; }
	int Height() const { return height; }

	void DrawRect(float x, float y, float w, float h, const glm::vec4& color);
	//the font is about 7 pixels tall, scale multiplies that. Shadow draws a dark copy behind for readability
	void DrawText(const std::string& text, float x, float y, float scale, const glm::vec4& color, bool shadow = true);
	void DrawTextCentered(const std::string& text, float centerX, float y, float scale, const glm::vec4& color, bool shadow = true);
	float TextWidth(const std::string& text, float scale) const;

private:
	struct UIVertex {
		glm::vec2 position;
		glm::vec4 color;
	};
	std::vector<UIVertex> vertices;

	VBO vbo;
	VAO vao;
	Shader* shaderPtr = nullptr;

	int width = 0;
	int height = 0;
};
