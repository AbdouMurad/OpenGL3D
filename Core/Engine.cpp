#include "Engine.h"

Application::Application(int w, int h, std::string n)
	:	width(w), height(h), name(n) {}

void Application::Run(Game& game) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window.Init(width, height, name, true);
	//fullscreen uses the monitor's resolution, not the requested one
	width = window.GetWindowWidth();
	height = window.GetWindowHeight();

	gladLoadGL();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FRAMEBUFFER_SRGB);

	//Face Culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	//turns off vsync -> only use to test fps
	glfwSwapInterval(0);

	std::cout << glGetString(GL_VERSION) << std::endl;

	glViewport(0, 0, width, height);

	renderer.init();
	debugRenderer.init();
	Collision::init();
	Input::Init(window.GetHandler());
	Input::SetCursorLocked(true);

	startGame(game);
	while (!window.ShouldClose()) {
		Time::Update();
		float dt = Time::DeltaTime();	
		if (dt > 0.1f) dt = 0.1f;	
		
		Input::Update();
		window.PollEvents();	
		if (Input::GetKeyDown(GLFW_KEY_Z)) debugRenderer.ToggleDebug();
		if (Input::GetKeyDown(GLFW_KEY_X)) renderer.ToggleShadowDebug();
		if (Input::GetKeyDown(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window.GetHandler(), GLFW_TRUE);
		if ((Input::GetKey(GLFW_KEY_LEFT_CONTROL) || Input::GetKey(GLFW_KEY_RIGHT_CONTROL)) && Input::GetKeyDown(GLFW_KEY_R)) {
			startGame(game);
			continue;
		}

		//glClearColor(0.529, 0.808f, 0.922f, 1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		game.Update(dt);
		game.scene.Update(dt);
		game.scene.Render(renderer);
		if (debugRenderer.IsDebug()) {
			game.scene.DebugRender(debugRenderer);
		}

		glfwSwapBuffers(window.GetHandler());
	}
	
	game.Shutdown();
	shutdown();
}

//(Re)builds the game's scene. Safe to call again at runtime to reset the game.
void Application::startGame(Game& game) {
	game.scene.Clear();
	game.Start(width, height);
	game.scene.Start();
}

void Application::shutdown() {
	window.Shutdown();
} 