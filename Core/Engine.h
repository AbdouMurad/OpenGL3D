#pragma once

#include "gl_core.h"
#include <string>

#include "Window.h"
#include "Input.h"
#include "Time_.h"
#include "Render/DebugRender.h"
#include "Render/UIRenderer.h"
#include "Core/Scene.h"
#include "SceneSerializer.h"

class Game {
private:
	
public:
	SceneSerializer serializer;
	Scene scene;

	//set to close the application at the end of the frame
	bool quitRequested = false;
	//skips scene updates (physics and components), Game::Update and rendering still run
	bool paused = false;
	glm::vec3 clearColor = glm::vec3(0.529f, 0.808f, 0.922f);

	virtual void Start(float width, float height) {};
	virtual void Update(float dt) {};
	//called after the scene is drawn, anything queued here is drawn on top
	virtual void RenderUI(UIRenderer& ui) {};
	virtual void Shutdown() {};
};

class Application {
public:
	Application(int width, int height, std::string applciationName);
	void Run(Game& game);

private:
	void shutdown();

	std::string name;
	int width = 1280;
	int height = 720;

	Window window;
	Renderer renderer;
	DebugRenderer debugRenderer;
	UIRenderer uiRenderer;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
};