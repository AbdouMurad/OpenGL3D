#include "Window.h"

bool Window::init(int w, int h, std::string windowName, bool fullscreen) {
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
void Window::swapBuffers() {
	glfwSwapBuffers(handler);
}
bool Window::shouldClose() {
	return glfwWindowShouldClose(handler);
}
void Window::pollEvents() {
	glfwPollEvents();
}
void Window::shutdown() {
	glfwDestroyWindow(handler);
	glfwTerminate();
}