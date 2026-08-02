#include "Engine.h"

Application::Application(int w, int h, std::string n)
	:	width(w), height(h), name(n) {}

void Application::Run(Game& game) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window.Init(width, height, name, false);

	gladLoadGL();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FRAMEBUFFER_SRGB);
	std::cout << glGetString(GL_VERSION) << std::endl;

	glViewport(0, 0, width, height);

	renderer.init();
	Collision::init();
	Input::Init(window.GetHandler());
	game.Start(width, height);
	game.scene.Start();
	while (!window.ShouldClose()) {
		Time::Update();
		float dt = Time::DeltaTime();
		// std::cout << "FPS: " << 1.0f / dt << "\n";
		
		Input::Update();
		window.PollEvents();	

		glClearColor(0.529, 0.808f, 0.922f, 1.0f);
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		game.scene.Update(dt);
		game.Update(dt);
		game.scene.Render(renderer);

		glfwSwapBuffers(window.GetHandler());
	}
	
	game.Shutdown();
	shutdown();
}

void Application::shutdown() {
	window.Shutdown();
} 