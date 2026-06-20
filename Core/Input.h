#pragma once

#include "gl_core.h"
#include <glm/glm.hpp>


class Input {
public:
	static void Init(GLFWwindow* window);
	static void Update();


	static bool GetKey(int key);
	static bool GetKeyDown(int key);
	static bool GetKeyUp(int key);

	static bool GetMouseButton(int button);
	static bool GetMouseButtonDown(int button);
	static bool GetMouseButtonUp(int button);
	static glm::vec2 GetMouseDelta();
	static glm::vec2 GetMousePos();

	static float GetScrollDelta();

	static void SetCursorLocked(bool locked);

private:
	static GLFWwindow* window;

	static bool keys[1024];
	static bool prevKeys[1024];

	static bool mouseButtons[8];
	static bool prevMouseButtons[8];

	static glm::vec2 mousePos;
	static glm::vec2 prevMousePos;
	static glm::vec2 mouseDelta;

	static float scrollDelta;
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void CursorCallback(GLFWwindow* window, double xpos, double ypos);
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

};
