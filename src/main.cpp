#include "Core/Engine.h"


class PlayerController : public Component {
public:
    void Start() override {};
    void Update(float dt) override {
        
        if (Input::GetKey(GLFW_KEY_W)) {
            owner->GetComponent<TransformComponent>()->translate({ 0,0, 15.0f * dt });
        }
        if (Input::GetKey(GLFW_KEY_S)) {
            owner->GetComponent<TransformComponent>()->translate({ 0,0, -15.0f * dt });
        }
        if (Input::GetKey(GLFW_KEY_A)) {
            owner->GetComponent<TransformComponent>()->rotate({ 0, 60.0f * dt, 0 });
        }
        if (Input::GetKey(GLFW_KEY_D)) {
            owner->GetComponent<TransformComponent>()->rotate({ 0,-60.0f * dt, 0 });
        }
    };
};

class MyGame : public Game {
public:
    void start(float width, float height) override {
        GameObject& object = scene.CreateObject();
        object.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/Crow_rig.gltf"));
        object.GetComponent<TransformComponent>()->setPosition({ 0,0,-12 });
        
        GameObject& camera = scene.CreateObject();
        camera.AddComponent<CameraComponent>(width, height);
        camera.GetComponent<CameraComponent>()->setViewport(width, height);

        GameObject& player = scene.CreateObject();
        player.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/cube.gltf"));
        player.GetComponent<TransformComponent>()->setSize({ 0.2f, 0.2f, 0.2f });
        player.AddComponent<PlayerController>();

        camera.GetComponent<TransformComponent>()->setPosition({ 0, 1.5f, 5.0f });
        camera.GetComponent<TransformComponent>()->parent = player.GetComponent<TransformComponent>();
        camera.GetComponent<TransformComponent>()->inheritScale = false;

        scene.SetMainCamera(camera);
    };

    void update(float dt) override {};


    void shutdown() override {};

    MyGame() {};
};


int main() {
    MyGame game;
    Application app = Application(1920, 1080, "GAME");
    app.run(game);
    return 0;
}