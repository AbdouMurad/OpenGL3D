#pragma once

#include "gl_core.h"

class Time {
public:
	static void update();
	
	static float deltaTime();


private:
	static float dt;
	static float lastFrame;
};