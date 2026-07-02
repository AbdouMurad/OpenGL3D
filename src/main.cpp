#include "Core/Engine.h"
#include "Render/Material.h"

class MyGame : public Game {
public:
    void start(int width, int height) override {
        
        // Vertices coordinates
        Vertex vertices[] = { //               COORDINATES           /            COLORS          /           NORMALS         /       TEXTURE COORDINATES    //
            Vertex{glm::vec3(-1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
            Vertex{glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
            Vertex{glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
            Vertex{glm::vec3(1.0f, 0.0f,  1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)}
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
        camera.setViewport(width, height); //TODO: Shouldnt need any camera calls like this in here
    };


    void update(float dt, Renderer& renderer) override {
        camera.updateMatrix(45.0f, 0.1f, 100.0f); //TODO: This too

        renderer.Draw(*floor, *floorMaterial, camera);

    };


    void shutdown() override {
        delete floor;
    };

    MyGame() : camera(0, 0, {0, 1, 3}) {};

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