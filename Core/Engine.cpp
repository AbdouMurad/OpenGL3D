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
	uiRenderer.init(width, height);
	Collision::init();
	Input::Init(window.GetHandler());
	
	game.Start(width, height);
	game.scene.Start();
	while (!window.ShouldClose() && !game.quitRequested) {
		Time::Update();
		float dt = Time::DeltaTime();	
		if (dt > 0.1f) dt = 0.1f;	
		
		Input::Update();
		window.PollEvents();	
		if (Input::GetKeyDown(GLFW_KEY_Z)) debugRenderer.ToggleDebug();

		//follow the window when it's resized or maximized, a minimized window reports 0x0 so keep the old size
		int framebufferWidth, framebufferHeight;
		glfwGetFramebufferSize(window.GetHandler(), &framebufferWidth, &framebufferHeight);
		if (framebufferWidth > 0 && framebufferHeight > 0 && (framebufferWidth != width || framebufferHeight != height)) {
			width = framebufferWidth;
			height = framebufferHeight;
			glViewport(0, 0, width, height);
			game.scene.GetMainCamera()->SetViewport(width, height);
			uiRenderer.Resize(width, height);
		}

		glClearColor(game.clearColor.r, game.clearColor.g, game.clearColor.b, 1.0f);
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		game.Update(dt);
		if (!game.paused) game.scene.Update(dt);
		game.scene.Render(renderer);
		if (debugRenderer.IsDebug()) {
			game.scene.DebugRender(debugRenderer);
		}
		game.RenderUI(uiRenderer);
		uiRenderer.Render();

		glfwSwapBuffers(window.GetHandler());
	}
	
	game.Shutdown();
	shutdown();
}

void Application::shutdown() {
	window.Shutdown();
} 