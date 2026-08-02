#pragma once

#include "Core/gl_core.h"

class Time {
public:
	static void Update();
	static float DeltaTime();

private:
	static float dt;
	static float lastFrame;
};