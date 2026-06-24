#pragma once
#include <memory>
#include "../render/Renderer.h"
#include "../world/World.h"
#include "Player.h"

class Game {
public:
    Game();
    void run();
private:
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<World> world;
    Player player;
    double lastTime;
    void processInput();

    static double scrollOffset;
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};
