#include "Core/Engine.h"


class PlayerController : public Component {
public:
    void Start() override {};
    void Update(float dt) override {
        glm::vec3 forward = owner->GetComponent<TransformComponent>()->Forward();
        glm::vec3 up = owner->GetComponent<TransformComponent>()->Up();
        if (Input::GetKey(GLFW_KEY_LEFT_CONTROL)) {
            owner->GetComponent<TransformComponent>()->translate(-up * 3.0f * dt);
        }
        if (Input::GetKey(GLFW_KEY_SPACE)) {
            owner->GetComponent<TransformComponent>()->translate(up * 3.0f * dt);
        }
        if (Input::GetKey(GLFW_KEY_W)) {
            owner->GetComponent<TransformComponent>()->translate(forward * 7.0f * dt);
        }
        if (Input::GetKey(GLFW_KEY_S)) {
            owner->GetComponent<TransformComponent>()->translate(-forward * 7.0f * dt);
        }
        if (Input::GetKey(GLFW_KEY_A)) {
            owner->GetComponent<TransformComponent>()->rotate({ 0, 50.0f * dt, 0 });
        }
        if (Input::GetKey(GLFW_KEY_D)) {
            owner->GetComponent<TransformComponent>()->rotate({ 0,-50.0f * dt, 0 });
        }
    };
};

class MyGame : public Game {
public:
    void start(float width, float height) override {
        GameObject& object = scene.CreateObject();
        object.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/Crow_rig.gltf"));
        object.GetComponent<TransformComponent>()->setPosition({ 0,0,-12 });

        GameObject& cube = scene.CreateObject();
        cube.GetComponent<TransformComponent>()->setPosition({ -3,0,-12 });
        cube.GetComponent<TransformComponent>()->setSize({ 0.1f,0.1f,0.1f });
        cube.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/cube.gltf"));

        GameObject& camera = scene.CreateObject();
        camera.AddComponent<CameraComponent>(width, height);
        camera.GetComponent<CameraComponent>()->setViewport(width, height);
        camera.AddComponent<PlayerController>();
        camera.AddComponent<PointLightComponent>(glm::vec3(1.0f, 1.0f, 1.0f), 15.0f, 100.0f);
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