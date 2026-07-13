#include "Core/Engine.h"
#include "Render/Material.h"

class MyGame : public Game {
public:
    void start(int width, int height) override {

        object.model = AssetManager::Get().LoadModel("assets/models/super.gltf");
        std::cout << AssetManager::Get().nextID << std::endl;
        object.transform.setPosition({ 0,0,0 });
        object.transform.setRotation({ 45,45,45 });

        //TODO: Shouldnt need any camera calls like this in here
        camera.setViewport(width, height); 
        camera.transform.setRotation({ 0,0,0 });
    };

    void update(float dt, Renderer& renderer) override {
        
        renderer.Draw(AssetManager::Get().GetModel(object.model), object.transform, camera);
        camera.updateMatrix(45.0f, 0.1f, 100.0f);
        if (Input::GetKey(GLFW_KEY_W)) {
            camera.transform.translate({ 0,0, 10.0f * dt });
        }
        if (Input::GetKey(GLFW_KEY_S)) {
            camera.transform.translate({ 0,0, -10.0f * dt });
        }
        if (Input::GetKey(GLFW_KEY_D)) {
            camera.transform.translate({ -10.0f * dt,0,0  });
        }
        if (Input::GetKey(GLFW_KEY_A)) {
            camera.transform.translate({ 10.0f * dt,0, 0 });
        }
        if (Input::GetKey(GLFW_KEY_SPACE)) {
            camera.transform.translate({ 0,10.0f * dt, 0 });
        }
        if (Input::GetKey(GLFW_KEY_V)) {
            camera.transform.translate({ 0,-10.0f * dt, 0 });
        }

        if (Input::GetKey(GLFW_KEY_R)) {
            camera.transform.rotate({ 0,200.0f * dt, 0 });
        }
        if (Input::GetKey(GLFW_KEY_T)) {
            camera.transform.rotate({ 0,-200.0f * dt, 0 });
        }
        //std::cout << "OBJ: " << object.transform << std::endl;
        /*std::cout << camera.transform << std::endl;*/
    };


    void shutdown() override {};

    MyGame() : camera(0, 0, { 0, 0, 0 }) {};

    
    Camera camera;

    GameObject object;
};


int main() {
    MyGame game;
    Application app = Application(1280, 900, "GAME");
    app.run(game);
    return 0;
}