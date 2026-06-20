#include "Core/Engine.h"

class MyGame : public Game {
public:
    void start() override {};
    void update(float dt) override {};
    void shutdown() override {};
};


int main() {
    MyGame game;
    Application app = Application(1280, 720, "GAME");
    app.run(game);
    return 0;
}