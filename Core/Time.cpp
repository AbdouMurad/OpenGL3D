#include "Core/Time_.h"

float Time::dt = 0.0f;
float Time::lastFrame = 0.0f;
float Time::fps = 0.0f;
float Time::frameTimeMs = 0.0f;
float Time::fpsTimer = 0.0f;
int Time::frameCount = 0;

void Time::Update() {
	float current = (float)glfwGetTime();

	dt = current - lastFrame;
	lastFrame = current;

	++frameCount;
	fpsTimer += dt;

	if (fpsTimer >= 0.5f) {
		fps = frameCount / fpsTimer;
		frameTimeMs = (fpsTimer * 1000.0f) / frameCount;
		std::cout << fps << " FPS\n";
		frameCount = 0;
		fpsTimer = 0.0f;
	}
}

float Time::DeltaTime() {
	return dt;
}

float Time::FPS() {
	return fps;
}
float Time::FrameTimeMS() {
	return frameTimeMs;
}
float Time::TotalTime() {
	return fpsTimer;
}