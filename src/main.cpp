#include "Core/Engine.h"


class PlayerController : public Component {
public:
    void Start() override {};
    void Update(float dt) override {
        float speedMult = 1.0f;
        if (Input::GetKey(GLFW_KEY_LEFT_SHIFT)) speedMult = 1.5f;
        glm::vec3 forward = owner->GetComponent<TransformComponent>()->Forward();
        glm::vec3 up = owner->GetComponent<TransformComponent>()->Up();
        if (Input::GetKey(GLFW_KEY_LEFT_CONTROL)) {
            owner->GetComponent<TransformComponent>()->Translate(-up * 3.0f * dt * speedMult);
        }
        if (Input::GetKey(GLFW_KEY_SPACE)) {
            owner->GetComponent<TransformComponent>()->Translate(up * 3.0f * dt * speedMult);
        }
        if (Input::GetKey(GLFW_KEY_W)) {
            owner->GetComponent<TransformComponent>()->Translate(forward * 7.0f * dt * speedMult);
        }
        if (Input::GetKey(GLFW_KEY_S)) {
            owner->GetComponent<TransformComponent>()->Translate(-forward * 7.0f * dt * speedMult);
        }
        if (Input::GetKey(GLFW_KEY_A)) {
            owner->GetComponent<TransformComponent>()->Rotate({ 0, 50.0f * dt * speedMult, 0 });
        }
        if (Input::GetKey(GLFW_KEY_D)) {
            owner->GetComponent<TransformComponent>()->Rotate({ 0,-50.0f * dt * speedMult, 0 });
        }
    };
};

class MyGame : public Game {
public:
    void Start(float width, float height) override {
        GameObject& object = scene.CreateObject();
        object.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/Crow_rig.gltf"));
        object.GetComponent<TransformComponent>()->SetPosition({ 0,0,-12 });
        object.AddComponent<RigidBodyComponent>(10.0f, true);
        
        
        GameObject& cube = scene.CreateObject();
        cube.GetComponent<TransformComponent>()->SetPosition({ 0,0,-12 });
        cube.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/sphere.gltf"));
        cube.AddComponent<RigidBodyComponent>();
        cube.AddComponent<ColliderComponent>(SHAPE::Sphere);
        cube.GetComponent<RigidBodyComponent>()->AddImpulse({-1, 0, 0});

        GameObject& cube2 = scene.CreateObject();
        cube2.GetComponent<TransformComponent>()->SetPosition({ -5,-5,-12 });
        cube2.GetComponent<TransformComponent>()->SetSize({ 7.0f,1.0f,7.0f });
        cube2.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/cube.gltf"));
        cube2.AddComponent<RigidBodyComponent>();
        cube2.GetComponent<RigidBodyComponent>()->ToggleStatic();
        cube2.AddComponent<ColliderComponent>(SHAPE::Box);
        Box* s = static_cast<Box*>(cube2.GetComponent<ColliderComponent>()->shape.get());
        s->halfExtent = glm::vec3(7, 1, 7);

        GameObject& camera = scene.CreateObject();
        camera.AddComponent<CameraComponent>(width, height);
        camera.GetComponent<CameraComponent>()->SetViewport(width, height);
        camera.AddComponent<PlayerController>();
        camera.AddComponent<PointLightComponent>(glm::vec3(1.0f, 1.0f, 1.0f), 15.0f, 25.0f);
        scene.SetMainCamera(camera);
    };

    void Update(float dt) override {};

    void Shutdown() override {};

    MyGame() {};
};


int main() {
    MyGame game;
    Application app = Application(1920, 1080, "GAME");
    app.Run(game);
    return 0;
}