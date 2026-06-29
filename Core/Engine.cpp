#include "Engine.h"

Application::Application(int w, int h, std::string n)
	:	width(w), height(h), name(n) {}
void Application::run(Game& game) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window.init(width, height, name, false);

	gladLoadGL();
	std::cout << glGetString(GL_VERSION) << std::endl;
	glViewport(0, 0, width, height);

	renderer.init();


	Input::Init(window.getHandler());
	game.start(width, height);
	while (!window.shouldClose()) {
		float dt = Time::deltaTime();
		Input::Update();
		window.pollEvents();	

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		game.update(dt, renderer);

		glfwSwapBuffers(window.getHandler());
	}
	
	game.shutdown();
	shutdown();
}
void Application::shutdown() {
	window.shutdown();
} 