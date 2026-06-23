#pragma once

#include "gl_core.h"
#include <string>
#include <iostream>

class Window {
public:
	bool init(int width, int height, std::string name, bool fullscreen);
	void swapBuffers();
	void pollEvents();
	bool shouldClose();
	void shutdown();

	int getWindowWidth() const { return width; }
	int getWindowHeight() const { return height; }

	GLFWwindow* getHandler() const { return handler; }
private:
	GLFWwindow* handler = nullptr;
	int width = 720;
	int height = 1280;
};