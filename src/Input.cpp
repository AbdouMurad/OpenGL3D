#include "Input.h"

GLFWwindow* Input::window = nullptr;

bool Input::keys[1024] = { false };
bool Input::prevKeys[1024] = { false };

bool Input::mouseButtons[8] = { false };
bool Input::prevMouseButtons[8] = { false };

glm::vec2 Input::mousePos = glm::vec2(0.0f, 0.0f);
glm::vec2 Input::prevMousePos = glm::vec2(0.0f, 0.0f);
glm::vec2 Input::mouseDelta = glm::vec2(0.0f, 0.0f);

float Input::scrollDelta = 0.0f;

void Input::Update() {
	for (int i = 0; i < 1024; ++i) {
		prevKeys[i] = keys[i];
	}
	for (int i = 0; i < 8; ++i) {
		prevMouseButtons[i] = mouseButtons[i];
	}
	mouseDelta = mousePos - prevMousePos;
	prevMousePos = mousePos;

	scrollDelta = 0.0f;
}

void Input::Init(GLFWwindow* win) {
	glfwSetKeyCallback(win, KeyCallback);
	glfwSetMouseButtonCallback(win, MouseButtonCallback);
	glfwSetCursorPosCallback(win, CursorCallback);
	glfwSetScrollCallback(win, ScrollCallback);
	window = win;
}

void Input::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key < 0 || key >= 1024) return;

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button < 0 || button >= 8) return;

	if (action == GLFW_PRESS)
		mouseButtons[button] = true;
	else if (action == GLFW_RELEASE)
		mouseButtons[button] = false;
}

void Input::CursorCallback(GLFWwindow* window, double xpos, double ypos)
{
	mousePos = glm::vec2(xpos, ypos);
}

void Input::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	scrollDelta = (float)yoffset;
}

bool Input::GetKey(int key) {
	return keys[key];
}
bool Input::GetKeyDown(int key) {
	return keys[key] && !prevKeys[key];
}
bool Input::GetKeyUp(int key) {
	return !keys[key] && prevKeys[key];
}
bool Input::GetMouseButton(int button) {
	return mouseButtons[button];
}
bool Input::GetMouseButtonDown(int button) {
	return mouseButtons[button] && !prevMouseButtons[button];
}
bool Input::GetMouseButtonUp(int button) {
	return !mouseButtons[button] && prevMouseButtons[button];
}
glm::vec2 Input::GetMouseDelta() {
	return mouseDelta;
}
glm::vec2 Input::GetMousePos() {
	return mousePos;
}
float Input::GetScrollDelta() {
	return scrollDelta;
}
void Input::SetCursorLocked(bool locked) {
	glfwSetInputMode(window, GLFW_CURSOR, locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}