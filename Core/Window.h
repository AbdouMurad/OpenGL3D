#pragma once

#include "gl_core.h"
#include <string>
#include <iostream>

class Window {
public:
	bool Init(int width, int height, std::string name, bool fullscreen);
	void SwapBuffers();
	void PollEvents();
	bool ShouldClose();
	void Shutdown();

	int GetWindowWidth() const { return width; }
	int GetWindowHeight() const { return height; }

	GLFWwindow* GetHandler() const { return handler; }
private:
	GLFWwindow* handler = nullptr;
	int width = 720;
	int height = 1280;
};