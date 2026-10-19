#include "Core/Engine.h"
#include "ThirdPersonCamera.h"
#include "ThirdPersonController.h"
#include "Components/GrassComponent.h"
#include <random>

class PlayerController : public Component {
    EventBus* eventBus;
    int groundOverlaps = 0;
public:
    PlayerController(EventBus* eb)
        :   eventBus(eb) {}


    void OnTriggerEnter(const TriggerEnterEvent& e) {
        if (e.trigger && e.trigger->getOwner() == owner) {
            ++groundOverlaps;
        }
    }
    void OnTriggerExit(const TriggerExitEvent& e) {
        if (e.trigger && e.trigger->getOwner() == owner) {
            groundOverlaps = std::max(0, groundOverlaps - 1);
        }
    }
    void Start() override {
        eventBus->Subscribe(this, &PlayerController::OnTriggerEnter);
        eventBus->Subscribe(this, &PlayerController::OnTriggerExit);
    };
    void Update(float dt) override {
        float speedMult = 1.0f;
        if (Input::GetKey(GLFW_KEY_LEFT_SHIFT)) speedMult = 1.5f;
        glm::vec3 right = owner->GetComponent<TransformComponent>()->Right();
        glm::vec3 forward = owner->GetComponent<TransformComponent>()->Forward();
        glm::vec3 up = owner->GetComponent<TransformComponent>()->Up();

        RigidBodyComponent* rb = owner->GetComponent<RigidBodyComponent>();
        glm::vec3 moveDir(0.0f);
        if (Input::GetKey(GLFW_KEY_W)) moveDir += forward;
        if (Input::GetKey(GLFW_KEY_S)) moveDir -= forward;
        if (Input::GetKey(GLFW_KEY_A)) moveDir -= right;
        if (Input::GetKey(GLFW_KEY_D)) moveDir += right;

        if (rb) {
            glm::vec3 velocity = rb->GetVelocity();
            if (glm::length2(moveDir) > 0.0f) {
                moveDir = glm::normalize(moveDir);
                velocity.x = moveDir.x * 7.0f * speedMult;
                velocity.z = moveDir.z * 7.0f * speedMult;
            }
            else {
                velocity.x = 0.0f;
                velocity.z = 0.0f;
            }
            if (Input::GetKey(GLFW_KEY_LEFT_CONTROL)) {
                velocity.y = -3.0f * speedMult;
            }
            rb->SetVelocity(velocity);
        }
        else {
            if (Input::GetKey(GLFW_KEY_LEFT_CONTROL)) {
                owner->GetComponent<TransformComponent>()->Translate(-up * 3.0f * dt * speedMult);
            }
            if (Input::GetKey(GLFW_KEY_W)) {
                owner->GetComponent<TransformComponent>()->Translate(forward * 7.0f * dt * speedMult);
            }
            if (Input::GetKey(GLFW_KEY_S)) {
                owner->GetComponent<TransformComponent>()->Translate(-forward * 7.0f * dt * speedMult);
            }
            if (Input::GetKey(GLFW_KEY_A)) {
                owner->GetComponent<TransformComponent>()->Translate(-right * 7.0f * dt * speedMult);
            }
            if (Input::GetKey(GLFW_KEY_D)) {
                owner->GetComponent<TransformComponent>()->Translate(right * 7.0f * dt * speedMult);
            }
        }

        if (Input::GetKeyDown(GLFW_KEY_SPACE) && groundOverlaps) {
            if (rb) {
                rb->AddImpulse(up * 6.0f);
            }
        }

        glm::vec2 mouseDelta = Input::GetMouseDelta();
        if (abs(mouseDelta.x) > 0.0001f)
            owner->GetComponent<TransformComponent>()->Rotate({ 0, -mouseDelta.x, 0 });
    };
};

//circles its object around a point at a fixed height in the sky, always facing the point
//on a directional light this sweeps the sun around the map: shadows turn like a sundial
//height in the sky and the starting point come from the object's rotation in the level file
class SunOrbit : public Component {
    glm::vec3 center;
    float distance;          //how far from the centre (only affects position, a directional light's direction is all that matters)
    float degreesPerSecond;  //orbit speed around the vertical axis
    float elevation = 0.0f;  //degrees above the horizon, read in Start()
    float angle = 0.0f;      //current position on the circle, degrees
public:
    SunOrbit(glm::vec3 center, float distance, float degreesPerSecond)
        :   center(center), distance(distance), degreesPerSecond(degreesPerSecond) {}

    void Start() override {
        //the light comes from the opposite of where it shines -> that's where the sun sits
        glm::vec3 forward = owner->GetComponent<TransformComponent>()->GetQuat() * glm::vec3(0, 0, -1);
        glm::vec3 towardsSun = -glm::normalize(forward);
        //kept below straight up: facing straight down has no defined "around the circle" direction
        elevation = glm::clamp(glm::degrees(std::asin(towardsSun.y)), 1.0f, 89.0f);
        angle = glm::degrees(std::atan2(towardsSun.z, towardsSun.x));
        Update(0.0f); //place it on the orbit right away
    }

    void Update(float dt) override {
        angle = std::fmod(angle + degreesPerSecond * dt, 360.0f);

        float a = glm::radians(angle);
        float e = glm::radians(elevation);
        //point on a circle around the vertical axis, raised by the elevation
        glm::vec3 offset(std::cos(e) * std::cos(a), std::sin(e), std::cos(e) * std::sin(a));
        glm::vec3 position = center + offset * distance;

        TransformComponent* transform = owner->GetComponent<TransformComponent>();
        transform->SetPosition(position);
        //quatLookAt turns -z (forward, the way the light shines) towards the centre
        transform->SetRotation(glm::quatLookAt(glm::normalize(center - position), glm::vec3(0, 1, 0)));
    }
};


class Shooter : public Game {
    GameObject* player = nullptr;
public:
    void Start(float width, float height) override {
        AssetManager::Get().LoadModel("assets/models/sphere_blue.gltf");
        
        GameObject& floor = scene.CreateObject();
        floor.GetComponent<TransformComponent>()->SetSize({ 15.0f,1.0f,15.0f });
        floor.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/cube.gltf"));
        RigidBodyComponent* floorRB = floor.AddComponent<RigidBodyComponent>();
        floorRB->ToggleStatic();
        ColliderComponent* floorCollider = floor.AddComponent<ColliderComponent>(SHAPE::Box);
        floorCollider->SetHalfExtent(glm::vec3(7.5, 0.5f, 7.5));


        player = &scene.CreateObject();
        RigidBodyComponent* playerRb = player->AddComponent<RigidBodyComponent>();
		playerRb->SetConstraint(RigidBodyConstraints::FreezeX | RigidBodyConstraints::FreezeY | RigidBodyConstraints::FreezeZ);
        //playerRb->AddConstraint(RigidBodyConstraints::FreezeX);
        //playerRb->AddConstraint(RigidBodyConstraints::FreezeZ);
        //playerRb->ToggleGravity();
        ColliderComponent* trigger = player->AddComponent<ColliderComponent>(SHAPE::Box);
        ColliderComponent* playerCollider = player->AddComponent<ColliderComponent>(SHAPE::Box);
        player->AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/cube.gltf"));
        player->AddComponent<PlayerController>(&scene.eventBus);
        trigger->SetTrigger(true);
        trigger->localOffset.position = glm::vec3(0, -0.5f, 0);
        trigger->SetHalfExtent(glm::vec3(0.25f, 0.125, 0.25f));
        player->GetComponent<TransformComponent>()->SetPosition({ 0,3,0 });

        //GameObject& box1 = scene.CreateObject();
        //RigidBodyComponent* rb1 = box1.AddComponent<RigidBodyComponent>();
        ////rb1->ToggleGravity();
        //rb1->SetMass(10.0f);
        //ColliderComponent* c1 = box1.AddComponent<ColliderComponent>(SHAPE::Box);
        //box1.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/cube.gltf"));
        //box1.GetComponent<TransformComponent>()->SetPosition({ 5,0,0 });


        GameObject& box2 = scene.CreateObject();
        RigidBodyComponent* rb2 = box2.AddComponent<RigidBodyComponent>();
        //rb2->ToggleGravity();
        rb2->SetMass(10.0f);
        box2.AddComponent<ColliderComponent>(SHAPE::Box);
        box2.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/cube.gltf"));
        box2.GetComponent<TransformComponent>()->SetPosition({ 4,1,0 });
        


        GameObject& camera = scene.CreateObject();
        CameraComponent* cameraComponent = camera.AddComponent<CameraComponent>(width, height);
        cameraComponent->SetViewport(width, height);
        camera.AddComponent<PointLightComponent>(glm::vec3(1.0f, 1.0f, 1.0f), 15.0f, 180.0f);
        camera.GetComponent<TransformComponent>()->parent = player->GetComponent<TransformComponent>();
        camera.GetComponent<TransformComponent>()->SetPosition({ 0, 2, 9.0f });
        scene.SetMainCamera(camera);

    }

    void Update(float dt) override {
        //Print(player->GetComponent<TransformComponent>()->GetPosition());
        if (Input::GetMouseButtonDown(0)) {
            GameObject& ball = scene.CreateObject();
            ball.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/sphere_blue.gltf"));
            RigidBodyComponent* rb = ball.AddComponent<RigidBodyComponent>();
            rb->SetMass(2);
            ColliderComponent* c = ball.AddComponent<ColliderComponent>(SHAPE::Sphere);
            c->SetRadius(0.15f);

            TransformComponent* transform = player->GetComponent<TransformComponent>();
            rb->AddImpulse(transform->Forward() * (75.0f * rb->GetMass()));
            ball.GetComponent<TransformComponent>()->SetPosition(transform->GetPosition() + transform->Forward());
            ball.GetComponent<TransformComponent>()->SetSize({ 0.3f,0.3f,0.3f });
        }
    }
};

class BallDisplay : public Game {
public:
    std::random_device rd;
    

    void Start(float width, float height) override {
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distrib(-6, 6);

        for (int i = 0; i < 250; i++) {
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
            cube.GetComponent<RigidBodyComponent>()->SetRestitution(0.98f);
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
        floor.GetComponent<ColliderComponent>()->SetHalfExtent(glm::vec3(7.5f, 0.05f, 7.5f));

        GameObject& ceiling = scene.CreateObject();
        ceiling.GetComponent<TransformComponent>()->SetPosition({ 0,10,0 });
        ceiling.GetComponent<TransformComponent>()->SetSize({ 15.0f,0.1f,15.0f });
        ceiling.AddComponent<RigidBodyComponent>();
        ceiling.GetComponent<RigidBodyComponent>()->ToggleStatic();
        ceiling.AddComponent<ColliderComponent>(SHAPE::Box);
        ceiling.GetComponent<ColliderComponent>()->SetHalfExtent(glm::vec3(7.5f, 0.05f, 7.5f));

        GameObject& wall = scene.CreateObject();
        wall.GetComponent<TransformComponent>()->SetPosition({ -7.5f, 2.5f, 0});
        wall.GetComponent<TransformComponent>()->SetSize({ 0.1f, 15.0f,15.0f });
        wall.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/cube.gltf"));
        wall.AddComponent<RigidBodyComponent>();
        wall.GetComponent<RigidBodyComponent>()->ToggleStatic();
        wall.AddComponent<ColliderComponent>(SHAPE::Box);
        wall.GetComponent<ColliderComponent>()->SetHalfExtent(glm::vec3(0.05f, 7.5f, 7.5f));

        GameObject& wall2 = scene.CreateObject();
        wall2.GetComponent<TransformComponent>()->SetPosition({ 0, 2.5f, -7.5f });
        wall2.GetComponent<TransformComponent>()->SetSize({ 15.0f, 15.0f,0.1f });
        wall2.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/cube.gltf"));
        wall2.AddComponent<RigidBodyComponent>();
        wall2.GetComponent<RigidBodyComponent>()->ToggleStatic();
        wall2.AddComponent<ColliderComponent>(SHAPE::Box);
        wall2.GetComponent<ColliderComponent>()->SetHalfExtent(glm::vec3(7.5f, 7.5f, 0.05f));

        GameObject& wall4 = scene.CreateObject();
        wall4.GetComponent<TransformComponent>()->SetPosition({ 0, 2.5f, 7.5f });
        wall4.GetComponent<TransformComponent>()->SetSize({ 15.0f, 15.0f,0.1f });
        wall4.AddComponent<RigidBodyComponent>();
        wall4.GetComponent<RigidBodyComponent>()->ToggleStatic();
        wall4.AddComponent<ColliderComponent>(SHAPE::Box);
        wall4.GetComponent<ColliderComponent>()->SetHalfExtent(glm::vec3(7.5f, 7.5f, 0.05f));

        GameObject& wall3 = scene.CreateObject();
        wall3.GetComponent<TransformComponent>()->SetPosition({ 7.5f, 2.5f, 0 });
        wall3.GetComponent<TransformComponent>()->SetSize({ 0.1f, 15.0f,15.0f });
        wall3.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/cube.gltf"));
        wall3.AddComponent<RigidBodyComponent>();
        wall3.GetComponent<RigidBodyComponent>()->ToggleStatic();
        wall3.AddComponent<ColliderComponent>(SHAPE::Box);
        wall3.GetComponent<ColliderComponent>()->SetHalfExtent(glm::vec3(0.05f, 7.5f, 7.5f));

        GameObject& camera = scene.CreateObject();
        camera.AddComponent<CameraComponent>(width, height);
        camera.GetComponent<CameraComponent>()->SetViewport(width, height);
        camera.AddComponent<PlayerController>(&scene.eventBus);
        camera.AddComponent<PointLightComponent>(glm::vec3(1.0f, 1.0f, 1.0f), 5.0f, 80.0f);
        camera.GetComponent<TransformComponent>()->SetPosition({ 0,1,20 });

        scene.SetMainCamera(camera);
    };

    void Update(float dt) override {

    };

    void Shutdown() override {};

    BallDisplay() {};
};


class LevelLoader : public Game {
    GameObject* player = nullptr;
    ThirdPersonCamera* followCamera = nullptr;
    float shootCooldown = 0.0f;
public:
    void Start(float width, float height) override {
        
        serializer.Load("assets/level/level1.json", scene);
        AssetManager::Get().LoadModel("assets/models/fireball.gltf");

        player = &scene.GetObject(2);

        GameObject& camera = scene.GetObject(3);
        //the level parents the camera to the player; the follow camera moves on its own instead
        camera.GetComponent<TransformComponent>()->parent = nullptr;
        CameraComponent* cameraComponent = camera.AddComponent<CameraComponent>(width, height);
        cameraComponent->SetViewport(width, height);
        followCamera = camera.AddComponent<ThirdPersonCamera>(&scene, player);
        scene.SetMainCamera(camera);

        player->AddComponent<ThirdPersonController>(&scene, followCamera);
        shootCooldown = 0.0f;

        //sun circles the middle of the map, one full turn every 36 s
        GameObject& sun = scene.GetObject(41);
        sun.AddComponent<SunOrbit>(glm::vec3(0.0f), 20.0f, 10.0f);
    }

    void Update(float dt) override {
        //fireballs go where the middle of the screen points, not just the way the player faces
        //(objects are created here, not in a component, because the scene is looping over its objects during component updates)
        shootCooldown -= dt;
        if (Input::GetMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT) && shootCooldown <= 0.0f) {
            shootCooldown = 0.15f;
            TransformComponent* transform = player->GetComponent<TransformComponent>();
            glm::vec3 playerPos = transform->GetPosition();

            //ray through the crosshair, started level with the player so nothing between the camera and the player gets hit
            glm::vec3 aimDir = followCamera->GetAimDirection();
            glm::vec3 aimOrigin = followCamera->GetAimOrigin();
            aimOrigin += aimDir * std::max(0.0f, glm::dot(playerPos - aimOrigin, aimDir));

            RaycastOptions options;
            options.ignore = player;
            options.maxDistance = 200.0f;
            RaycastHit hit;
            glm::vec3 aimPoint = scene.Raycast(aimOrigin, aimDir, hit, options) ? hit.point : aimOrigin + aimDir * 200.0f;

            //spawn just in front of the player, towards the aim point
            glm::vec3 handPos = playerPos + glm::vec3(0.0f, 0.3f, 0.0f);
            glm::vec3 flat = aimPoint - handPos;
            flat.y = 0.0f;
            if (glm::length2(flat) > 1e-6f) handPos += glm::normalize(flat) * 0.9f;
            glm::vec3 shotDir = glm::normalize(aimPoint - handPos);

            GameObject& ball = scene.CreateObject();
            ball.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel("assets/models/fireball.gltf"));
            RigidBodyComponent* rb = ball.AddComponent<RigidBodyComponent>();
            rb->SetMass(15);
            ColliderComponent* c = ball.AddComponent<ColliderComponent>(SHAPE::Sphere);
            c->SetRadius(0.15f);
            rb->AddImpulse(shotDir * (40.0f * rb->GetMass()));
            ball.GetComponent<TransformComponent>()->SetPosition(handPos);
            ball.GetComponent<TransformComponent>()->SetSize({ 0.3f,0.3f,0.3f });

            followCamera->AddTrauma(0.12f);
        }
    }
};

//makes a point light's brightness waver a little, like a flame. the light stays where it is
class LanternFlicker : public Component {
    PointLightComponent* light = nullptr;
    float baseIntensity = 0.0f;
    float time;
public:
    LanternFlicker(float startTime) : time(startTime) {} //different start times -> lanterns don't flicker in sync

    void Start() override {
        light = owner->GetComponent<PointLightComponent>();
        if (light) baseIntensity = light->intensity;
    }
    void Update(float dt) override {
        if (!light) return;
        time += dt;
        light->intensity = baseIntensity * (0.92f + 0.05f * std::sin(time * 7.3f) + 0.03f * std::sin(time * 13.7f + 1.1f));
    }
};

//open grassy field in a grayscale world. colour only exists inside the lanterns' light
//balls can be pushed around: roll a gray one into a lantern's light and it shows its colour
class GrayField : public Game {
    static constexpr float fieldHalfSize = 50.0f; //walkable square is 100 x 100 m
    std::vector<glm::vec2> lanternSpots;

    struct LanternColor { const char* orb; glm::vec3 color; };
    static constexpr LanternColor lanternColors[6] = {
        { "amber",   { 1.0f, 0.55f, 0.20f } },
        { "teal",    { 0.20f, 0.90f, 0.75f } },
        { "magenta", { 0.95f, 0.25f, 0.75f } },
        { "lime",    { 0.55f, 1.0f, 0.20f } },
        { "sky",     { 0.30f, 0.55f, 1.0f } },
        { "gold",    { 1.0f, 0.85f, 0.35f } },
    };
    static constexpr const char* paints[8] = { "red", "orange", "yellow", "green", "cyan", "blue", "purple", "pink" };

    static std::string ModelPath(const std::string& name) { return "assets/models/grayfield/" + name + ".gltf"; }

    void CreateLantern(glm::vec2 spot, int colorIndex, float flickerOffset) {
        const LanternColor& lantern = lanternColors[colorIndex % 6];

        GameObject& pole = scene.CreateObject();
        pole.SetName("lantern_pole");
        pole.GetComponent<TransformComponent>()->SetPosition({ spot.x, 1.2f, spot.y });
        pole.GetComponent<TransformComponent>()->SetSize({ 0.15f, 2.4f, 0.15f });
        pole.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel(ModelPath("pole")));
        pole.AddComponent<RigidBodyComponent>()->ToggleStatic();
        pole.AddComponent<ColliderComponent>(SHAPE::Box)->SetHalfExtent({ 0.1f, 1.2f, 0.1f });

        //the orb glows in its own colour (emissive, never grayed out) and carries the light
        GameObject& orb = scene.CreateObject();
        orb.SetName(std::string("lantern_") + lantern.orb);
        orb.GetComponent<TransformComponent>()->SetPosition({ spot.x, 2.65f, spot.y });
        orb.GetComponent<TransformComponent>()->SetSize(glm::vec3(0.35f));
        orb.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel(ModelPath(std::string("orb_") + lantern.orb)));
        //light tinted towards the orb's colour, but not fully: objects should still show their own colours
        glm::vec3 lightColor = glm::mix(glm::vec3(1.0f), lantern.color, 0.6f);
        orb.AddComponent<PointLightComponent>(lightColor, 5.0f, 9.0f);
        orb.AddComponent<LanternFlicker>(flickerOffset);

        lanternSpots.push_back(spot);
    }

    //static painted block, random size and turn, standing on the ground
    void CreateBlock(glm::vec2 spot, std::mt19937& rng) {
        std::uniform_real_distribution<float> footprint(0.8f, 2.2f), tall(1.0f, 2.8f), turn(0.0f, 90.0f);
        std::uniform_int_distribution<int> paint(0, 7);
        glm::vec3 size(footprint(rng), tall(rng), footprint(rng));

        GameObject& block = scene.CreateObject();
        block.SetName("block");
        TransformComponent* transform = block.GetComponent<TransformComponent>();
        transform->SetPosition({ spot.x, size.y * 0.5f, spot.y });
        transform->SetSize(size);
        transform->SetRotation(glm::vec3(0.0f, turn(rng), 0.0f));
        block.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel(ModelPath(std::string("block_") + paints[paint(rng)])));
        block.AddComponent<RigidBodyComponent>()->ToggleStatic();
        block.AddComponent<ColliderComponent>(SHAPE::Box)->SetHalfExtent(size * 0.5f);
    }

    //light painted ball the player can push around
    void CreateBall(glm::vec2 spot, std::mt19937& rng) {
        std::uniform_real_distribution<float> radiusRange(0.3f, 0.6f);
        std::uniform_int_distribution<int> paint(0, 7);
        float radius = radiusRange(rng);

        GameObject& ball = scene.CreateObject();
        ball.SetName("ball");
        ball.GetComponent<TransformComponent>()->SetPosition({ spot.x, radius + 0.05f, spot.y });
        ball.GetComponent<TransformComponent>()->SetSize(glm::vec3(radius * 2.0f)); //model is 1 across
        ball.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel(ModelPath(std::string("ball_") + paints[paint(rng)])));
        RigidBodyComponent* rb = ball.AddComponent<RigidBodyComponent>();
        rb->SetMass(1.5f);
        rb->SetRestitution(0.3f);
        ball.AddComponent<ColliderComponent>(SHAPE::Sphere)->SetRadius(radius);
    }

    //somewhere for a block/ball that isn't on top of the spawn point or a lantern pole
    bool IsFree(glm::vec2 spot, float clearance) const {
        if (glm::length(spot) < 3.0f + clearance) return false;
        for (const glm::vec2& lantern : lanternSpots)
            if (glm::distance(spot, lantern) < 1.0f + clearance) return false;
        return true;
    }

public:
    void Start(float width, float height) override {
        lanternSpots.clear();
        std::mt19937 rng(7); //fixed seed: same layout every run (and after Ctrl+R)
        std::uniform_real_distribution<float> unit(0.0f, 1.0f);

        scene.environment.grayWorld = true;
        scene.environment.skyColor = glm::vec3(0.035f, 0.037f, 0.042f);
        scene.environment.fogDensity = 0.028f;

        //---- ground ----
        //one big collider, but the visible ground is 10 m tiles: the renderer picks lights per object by its centre,
        //so one 100 m mesh would only get the lanterns near the middle
        GameObject& ground = scene.CreateObject();
        ground.SetName("ground");
        ground.GetComponent<TransformComponent>()->SetPosition({ 0.0f, -0.5f, 0.0f });
        ground.AddComponent<RigidBodyComponent>()->ToggleStatic();
        ground.AddComponent<ColliderComponent>(SHAPE::Box)->SetHalfExtent({ fieldHalfSize, 0.5f, fieldHalfSize });

        ModelHandle groundTile = AssetManager::Get().LoadModel(ModelPath("ground"));
        constexpr float tileSize = 10.0f;
        for (float x = -fieldHalfSize + tileSize * 0.5f; x < fieldHalfSize; x += tileSize) {
            for (float z = -fieldHalfSize + tileSize * 0.5f; z < fieldHalfSize; z += tileSize) {
                GameObject& tile = scene.CreateObject();
                tile.GetComponent<TransformComponent>()->SetPosition({ x, -0.1f, z });
                tile.GetComponent<TransformComponent>()->SetSize({ tileSize, 0.2f, tileSize });
                tile.AddComponent<MeshRenderer>(groundTile);
            }
        }

        //---- moon: dim, white, casts the only shadows ----
        GameObject& moon = scene.CreateObject();
        moon.SetName("moon");
        moon.GetComponent<TransformComponent>()->SetRotation(glm::vec3(-35.0f, 40.0f, 0.0f));
        moon.AddComponent<DirectionalLightComponent>(glm::vec3(1.0f), 0.6f);

        //---- lanterns: one close to the spawn, the rest on a jittered grid across the field ----
        CreateLantern({ 4.0f, -6.0f }, 0, 0.0f);
        int colorIndex = 1;
        for (float gx = -33.0f; gx <= 33.0f; gx += 22.0f) {
            for (float gz = -33.0f; gz <= 33.0f; gz += 22.0f) {
                glm::vec2 spot(gx + (unit(rng) - 0.5f) * 10.0f, gz + (unit(rng) - 0.5f) * 10.0f);
                if (glm::distance(spot, glm::vec2(4.0f, -6.0f)) < 10.0f) continue; //too close to the first one
                CreateLantern(spot, colorIndex++, unit(rng) * 10.0f);
            }
        }

        //---- things to find: a few inside every lantern's light, more out in the dark ----
        for (size_t i = 0; i < lanternSpots.size(); ++i) {
            glm::vec2 lantern = lanternSpots[i];
            for (int k = 0; k < 3; ++k) {
                float angle = unit(rng) * 6.2831853f;
                glm::vec2 spot = lantern + glm::vec2(std::cos(angle), std::sin(angle)) * glm::mix(2.5f, 5.5f, unit(rng));
                if (!IsFree(spot, 1.2f)) continue;
                if (k == 2) CreateBall(spot, rng);
                else CreateBlock(spot, rng);
            }
        }
        for (int i = 0; i < 70; ++i) {
            glm::vec2 spot((unit(rng) * 2.0f - 1.0f) * (fieldHalfSize - 3.0f), (unit(rng) * 2.0f - 1.0f) * (fieldHalfSize - 3.0f));
            if (!IsFree(spot, 1.2f)) continue;
            if (unit(rng) < 0.55f) CreateBlock(spot, rng);
            else CreateBall(spot, rng);
        }

        //---- player ----
        GameObject& player = scene.CreateObject();
        player.SetName("player");
        player.GetComponent<TransformComponent>()->SetPosition({ 0.0f, 1.0f, 0.0f });
        player.AddComponent<MeshRenderer>(AssetManager::Get().LoadModel(ModelPath("player")));
        RigidBodyComponent* playerRb = player.AddComponent<RigidBodyComponent>();
        playerRb->SetConstraint(RigidBodyConstraints::FreezeX | RigidBodyConstraints::FreezeY | RigidBodyConstraints::FreezeZ);
        player.AddComponent<ColliderComponent>(SHAPE::Box)->SetHalfExtent(glm::vec3(0.5f));

        //---- camera ----
        GameObject& camera = scene.CreateObject();
        camera.SetName("camera");
        CameraComponent* cameraComponent = camera.AddComponent<CameraComponent>(width, height);
        cameraComponent->SetViewport(width, height);
        cameraComponent->farPlane = 150.0f;
        ThirdPersonCamera* followCamera = camera.AddComponent<ThirdPersonCamera>(&scene, &player);
        scene.SetMainCamera(camera);
        player.AddComponent<ThirdPersonController>(&scene, followCamera);

        //---- grass over the whole field, parted by the player ----
        GrassSettings grass;
        grass.halfSize = glm::vec2(fieldHalfSize);
        grass.density = 60.0f;  //~600k blades over the field, thinned out with distance by the renderer
        grass.bladeWidth = 0.09f;
        GameObject& field = scene.CreateObject();
        field.SetName("grass");
        field.AddComponent<GrassComponent>(grass, &player);
    }
};

int main() {
    GrayField game;
    //LevelLoader game;
    //BallDisplay game;
    //Shooter game;4
    Application app = Application(1920, 1080, "GAME");
    app.Run(game);
    return 0;
}