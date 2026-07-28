#include "Window.h"

bool Window::Init(int w, int h, std::string windowName, bool fullscreen) {
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	if (fullscreen) {
		handler = glfwCreateWindow(mode->width, mode->height, windowName.c_str(), monitor, NULL);
		height = mode->height;
		width = mode->width;
	}
	else {
		handler = glfwCreateWindow(w, h, windowName.c_str(), NULL, NULL);
		width = w;
		height = h;
	}
	if (handler == NULL) {
		std::cout << "Failed to creatWe GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(handler);

	return true;
}
void Window::SwapBuffers() {
	glfwSwapBuffers(handler);
}
bool Window::ShouldClose() {
	return glfwWindowShouldClose(handler);
}
void Window::PollEvents() {
	glfwPollEvents();
}
void Window::Shutdown() {
	glfwDestroyWindow(handler);
	glfwTerminate();
}