#include "Core/Engine.h"
#include "Render/Material.h"

class MyGame : public Game {
public:
    void start(int width, int height) override {
        
        Vertex vertices[] = 
        { //               COORDINATES           /            NORMALS          /           COLORS         /       TEXTURE COORDINATES    //
            Vertex{glm::vec3(-1.0f, 0.0f,  1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
            Vertex{glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
            Vertex{glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
            Vertex{glm::vec3(1.0f, 0.0f,  1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)}
        };

        // Indices for vertices order
        GLuint indices[] =
        {
            0, 1, 2,
            0, 2, 3
        };
        
        std::vector <Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
        std::vector <GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));

        floor = new Mesh(verts, ind);
        floorMaterial = new UnlitMaterial("assets/textures/planks.png");
       
        //TODO: Shouldnt need any camera calls like this in here
        camera.setViewport(width, height); 
        camera.transform.setRotation({ -15,-90,0 });
         
    };


    void update(float dt, Renderer& renderer) override {

        renderer.Draw(*floor, *floorMaterial, camera);
        camera.updateMatrix(45.0f, 0.1f, 100.0f);
        if (Input::GetKey(GLFW_KEY_W)) {
            camera.transform.translate({ 0,0, -10.0f * dt });
            std::cout << dt << std::endl;
            std::cout << camera.transform << std::endl;
        }
        else if (Input::GetKey(GLFW_KEY_S)) {
            camera.transform.translate({ 0,0, 10.0f * dt });
            std::cout << dt << std::endl;
            std::cout << camera.transform << std::endl;
        }
        else if (Input::GetKey(GLFW_KEY_D)) {
            camera.transform.rotate({ 0,100.0f * dt, 0 });
            std::cout << dt << std::endl;
            std::cout << camera.transform << std::endl;
        }
        else if (Input::GetKey(GLFW_KEY_A)) {
            camera.transform.rotate({ 0,-100.0f * dt, 0 });
            std::cout << dt << std::endl;
            std::cout << camera.transform << std::endl;
        }
    };


    void shutdown() override {
        delete floor;
        delete floorMaterial;
    };

    MyGame() : camera(0, 0, { 0, 1, 3 }) {};

    Mesh* floor = nullptr;
    Material* floorMaterial = nullptr;
    
    Camera camera;
    

};


int main() {
    MyGame game;
    Application app = Application(1280, 900, "GAME");
    app.run(game);
    return 0;
}