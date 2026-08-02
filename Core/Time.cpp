#include "Core/Time_.h"

float Time::dt = 0.0f;
float Time::lastFrame = 0.0f;

void Time::Update() {
	float current = (float)glfwGetTime();

	dt = current - lastFrame;
	lastFrame = current;
}

float Time::DeltaTime() {
	return dt;
}