#include "Core/Engine.h"

class MyGame : public Game {
public:
    void start(float width, float height) override {

        object.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/super.gltf"));
        object.AddComponent<TransformComponent>();

        camera.AddComponent<TransformComponent>();
        camera.AddComponent<CameraComponent>(width, height);
        camera.GetComponent<CameraComponent>()->setViewport(width, height);

        TransformComponent* transform = object.GetComponent<TransformComponent>();
        transform->setPosition({ 0,0,0 });
        transform->setRotation({ 0,0,0 });

    };

    void update(float dt, Renderer& renderer) override {
        renderer.BeginFrame(*camera.GetComponent<CameraComponent>());
        object.GetComponent<MeshRenderer>()->Render(renderer);
        camera.GetComponent<CameraComponent>()->updateMatrix(45.0f, 0.1f, 100.0f);

        if (Input::GetKey(GLFW_KEY_W)) {
            camera.GetComponent<TransformComponent>()->translate({0,0, 8.0f * dt});
        }
        if (Input::GetKey(GLFW_KEY_S)) {
            camera.GetComponent<TransformComponent>()->translate({ 0,0, -8.0f * dt });
        }
        if (Input::GetKey(GLFW_KEY_D)) {
            camera.GetComponent<TransformComponent>()->translate({ -8.0f * dt,0,0  });
        }
        if (Input::GetKey(GLFW_KEY_A)) {
            camera.GetComponent<TransformComponent>()->translate({ 8.0f * dt,0, 0 });
        }
        if (Input::GetKey(GLFW_KEY_SPACE)) {
            camera.GetComponent<TransformComponent>()->translate({ 0,8.0f * dt, 0 });
        }
        if (Input::GetKey(GLFW_KEY_V)) {
            camera.GetComponent<TransformComponent>()->translate({ 0,-8.0f * dt, 0 });
        }

        if (Input::GetKey(GLFW_KEY_R)) {
            camera.GetComponent<TransformComponent>()->rotate({ 0, 100.0f * dt, 0 });
        }
        if (Input::GetKey(GLFW_KEY_T)) {
            camera.GetComponent<TransformComponent>()->rotate({ 0,-100.0f * dt, 0 });
        }
    };


    void shutdown() override {};

    MyGame() {};

    GameObject object;
    GameObject camera;
};


int main() {
    MyGame game;
    Application app = Application(1280, 900, "GAME");
    app.run(game);
    return 0;
}