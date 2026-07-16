#pragma once

#include "gl_core.h"
#include <string>

#include "Window.h"
#include "Input.h"
#include "Time_.h"
#include "Render/Renderer.h"

class Game {
private:
	
public:
	virtual void start(float width, float height) {};
	virtual void update(float dt, Renderer& renderer) {};
	virtual void shutdown() {};
};

class Application {
public:
	Application(int width, int height, std::string applciationName);
	void run(Game& game);

private:
	void shutdown();

	std::string name;
	int width = 1280;
	int height = 720;

	Window window;
	Renderer renderer;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
};