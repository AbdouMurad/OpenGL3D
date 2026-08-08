#include "Core/Engine.h"
#include <random>

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

class BallDisplay : public Game {
public:
    std::random_device rd;
    

    void Start(float width, float height) override {
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distrib(-6, 6);

        for (int i = 0; i < 150; i++) {
            GameObject& cube = scene.CreateObject();
            cube.GetComponent<TransformComponent>()->SetPosition(glm::vec3((float)distrib(gen), (float)distrib(gen)/2.0f, (float)distrib(gen)));
            if (i % 3 == 0) {
                cube.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/sphere_red.gltf"));
            }
            else if (i % 3 == 1) {
                cube.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/sphere_blue.gltf"));
            }
            else if (i % 3 == 2) {
                cube.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/sphere_yellow.gltf"));
            }
            cube.AddComponent<RigidBodyComponent>();
            cube.GetComponent<RigidBodyComponent>()->AddImpulse(glm::vec3((float)distrib(gen), 0, (float)distrib(gen)) * 0.5f);
            cube.AddComponent<ColliderComponent>(SHAPE::Sphere);

        }

        GameObject& floor = scene.CreateObject();
        floor.GetComponent<TransformComponent>()->SetPosition({ 0,-5,0 });
        floor.GetComponent<TransformComponent>()->SetSize({ 15.0f,0.1f,15.0f });
        floor.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/cube.gltf"));
        floor.AddComponent<RigidBodyComponent>();
        floor.GetComponent<RigidBodyComponent>()->ToggleStatic();
        floor.AddComponent<ColliderComponent>(SHAPE::Box);
        Box* floorCollider = static_cast<Box*>(floor.GetComponent<ColliderComponent>()->shape.get());
        floorCollider->halfExtent = glm::vec3(7.5f, 0.05f, 7.5f);

        GameObject& ceiling = scene.CreateObject();
        ceiling.GetComponent<TransformComponent>()->SetPosition({ 0,10,0 });
        ceiling.GetComponent<TransformComponent>()->SetSize({ 15.0f,0.1f,15.0f });
        ceiling.AddComponent<RigidBodyComponent>();
        ceiling.GetComponent<RigidBodyComponent>()->ToggleStatic();
        ceiling.AddComponent<ColliderComponent>(SHAPE::Box);
        Box* ceilingCollider = static_cast<Box*>(ceiling.GetComponent<ColliderComponent>()->shape.get());
        ceilingCollider->halfExtent = glm::vec3(7.5f, 0.05f, 7.5f);

        GameObject& wall = scene.CreateObject();
        wall.GetComponent<TransformComponent>()->SetPosition({ -7.5f, 2.5f, 0});
        wall.GetComponent<TransformComponent>()->SetSize({ 0.1f, 15.0f,15.0f });
        wall.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/cube.gltf"));
        wall.AddComponent<RigidBodyComponent>();
        wall.GetComponent<RigidBodyComponent>()->ToggleStatic();
        wall.AddComponent<ColliderComponent>(SHAPE::Box);
        Box* wallCollider1 = static_cast<Box*>(wall.GetComponent<ColliderComponent>()->shape.get());
        wallCollider1->halfExtent = glm::vec3(0.05f, 7.5f, 7.5f);

        GameObject& wall2 = scene.CreateObject();
        wall2.GetComponent<TransformComponent>()->SetPosition({ 0, 2.5f, -7.5f });
        wall2.GetComponent<TransformComponent>()->SetSize({ 15.0f, 15.0f,0.1f });
        wall2.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/cube.gltf"));
        wall2.AddComponent<RigidBodyComponent>();
        wall2.GetComponent<RigidBodyComponent>()->ToggleStatic();
        wall2.AddComponent<ColliderComponent>(SHAPE::Box);
        Box* wallCollider2 = static_cast<Box*>(wall2.GetComponent<ColliderComponent>()->shape.get());
        wallCollider2->halfExtent = glm::vec3(7.5f, 7.5f, 0.05f);

        GameObject& wall4 = scene.CreateObject();
        wall4.GetComponent<TransformComponent>()->SetPosition({ 0, 2.5f, 7.5f });
        wall4.GetComponent<TransformComponent>()->SetSize({ 15.0f, 15.0f,0.1f });
        wall4.AddComponent<RigidBodyComponent>();
        wall4.GetComponent<RigidBodyComponent>()->ToggleStatic();
        wall4.AddComponent<ColliderComponent>(SHAPE::Box);
        Box* wallCollider4 = static_cast<Box*>(wall4.GetComponent<ColliderComponent>()->shape.get());
        wallCollider4->halfExtent = glm::vec3(7.5f, 7.5f, 0.05f);

        GameObject& wall3 = scene.CreateObject();
        wall3.GetComponent<TransformComponent>()->SetPosition({ 7.5f, 2.5f, 0 });
        wall3.GetComponent<TransformComponent>()->SetSize({ 0.1f, 15.0f,15.0f });
        wall3.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/cube.gltf"));
        wall3.AddComponent<RigidBodyComponent>();
        wall3.GetComponent<RigidBodyComponent>()->ToggleStatic();
        wall3.AddComponent<ColliderComponent>(SHAPE::Box);
        Box* wallCollider3 = static_cast<Box*>(wall3.GetComponent<ColliderComponent>()->shape.get());
        wallCollider3->halfExtent = glm::vec3(0.05f, 7.5f, 7.5f);

        GameObject& camera = scene.CreateObject();
        camera.AddComponent<CameraComponent>(width, height);
        camera.GetComponent<CameraComponent>()->SetViewport(width, height);
        camera.AddComponent<PlayerController>();
        camera.AddComponent<PointLightComponent>(glm::vec3(1.0f, 1.0f, 1.0f), 75.0f, 650.0f);
        camera.GetComponent<TransformComponent>()->SetPosition({ 0,1,20 });

        scene.SetMainCamera(camera);
    };

    void Update(float dt) override {

    };

    void Shutdown() override {};

    BallDisplay() {};
};


int main() {
    BallDisplay game;
    Application app = Application(1920, 1080, "GAME");
    app.Run(game);
    return 0;
}