#include "Core/Engine.h"
#include<filesystem>
namespace fs = std::filesystem;
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
        std::string parentDir = (fs::current_path().fs::path::parent_path()).string();
        std::cout << parentDir << std::endl;
        Texture textures[]
        {
            Texture("textures/planks.png", "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE),
            Texture("textures/planksSpec.png", "specular", 1, GL_RED, GL_UNSIGNED_BYTE)
        };
        
        std::vector <Vertex> verts(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
        std::vector <GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));
        std::vector <Texture> tex(textures, textures + sizeof(textures) / sizeof(Texture));
        floor = new Mesh(verts, ind, tex);
        
        camera.setViewport(width, height);
    };
    void update(float dt, Renderer& renderer) override {
        camera.updateMatrix(45.0f, 0.1f, 100.0f);
        renderer.Draw(*floor, *renderer.getShader(ShaderType::DEFAULT), camera);

    };
    void shutdown() override {
        delete floor;
    };

    MyGame() : camera(0, 0, {0, 1, 2}) {};
    Mesh* floor = nullptr;
    Camera camera;


};


int main() {
    MyGame game;
    Application app = Application(1980, 1080, "GAME");
    app.run(game);
    return 0;
}