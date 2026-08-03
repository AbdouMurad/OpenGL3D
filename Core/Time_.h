#pragma once

#include "Core/gl_core.h"
#include <iostream>
class Time {
public:
	static void Update();
	static float DeltaTime();

	static float FPS();
	static float FrameTimeMS();
	static float TotalTime();

private:
	static float dt;
	static float lastFrame;

	static float fps;
	static float frameTimeMs;

	static float fpsTimer;
	static int frameCount;
};