#include "Window.h"

bool Window::init(int w, int h, std::string windowName, bool fullscreen) {
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	if (fullscreen) {
		window = glfwCreateWindow(mode->width, mode->height, windowName, monitor, NULL);
		height = mode->height;
		width = mode->width;
	}
	else {
		window = glfwCreateWindow(w, h, windowName, NULL, NULL);
		width = w;
		height = h;
	}
	if (window == NULL) {
		std::cout << "Failed to creatWe GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	return true;
}
void Window::swapBuffers() {
	glfwSwapBuffers();
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