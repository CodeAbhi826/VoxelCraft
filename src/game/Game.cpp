#include "Game.h"
#include "../render/MeshBuilder.h"
#include <GLFW/glfw3.h>
#include <string>
#include <cmath>
#include <algorithm>

double Game::scrollOffset = 0.0;

void Game::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    scrollOffset += yoffset;
}

void Game::cursorCallback(GLFWwindow* window, double xpos, double ypos) {
    Game* self = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (self) self->player.onMouseMove(xpos, ypos);
}

Game::Game()
: renderer(std::make_unique<Renderer>(1280, 720)),
  world(std::make_unique<World>(12345ull))
{
    player.position = Vec3(0, 100, 0);
    lastTime = glfwGetTime();
    renderer->setRenderDistance(settings.renderDistance);
    world->updatePlayerPosition(0, 0, settings.renderDistance);
    glfwSetScrollCallback(renderer->window, scrollCallback);
    glfwSetWindowUserPointer(renderer->window, this);
    glfwSetCursorPosCallback(renderer->window, cursorCallback);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(renderer->window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}

void Game::run() {
    int fpsFrames = 0;
    double fpsLast = lastTime;
    std::string fpsStr = "FPS: 0";

    double frameTimeAvg = 0.016;
    double lowFpsTimer = 0, highFpsTimer = 0;

    while (!glfwWindowShouldClose(renderer->window)) {
        double now = glfwGetTime();
        float dt = float(now - lastTime);
        lastTime = now;
        if (dt > 0.1f) dt = 0.1f;

        frameTimeAvg = frameTimeAvg * 0.9 + dt * 0.1;
        double fps = 1.0 / frameTimeAvg;

        // OPT 7: adaptive render distance
        if (fps < 50) {
            lowFpsTimer += dt;
            highFpsTimer = 0;
            if (lowFpsTimer > 2.0 && settings.renderDistance > 3) {
                settings.renderDistance--;
                renderer->setRenderDistance(settings.renderDistance);
                world->updatePlayerPosition((int)player.position.x, (int)player.position.z, settings.renderDistance);
                lowFpsTimer = 0;
            }
        } else if (fps > 80) {
            highFpsTimer += dt;
            lowFpsTimer = 0;
            if (highFpsTimer > 5.0 && settings.renderDistance < 12) {
                settings.renderDistance++;
                renderer->setRenderDistance(settings.renderDistance);
                highFpsTimer = 0;
            }
        } else {
            lowFpsTimer = 0;
            highFpsTimer = 0;
        }

        player.processInput(renderer->window, dt);

        if (scrollOffset != 0.0) {
            int dir = scrollOffset > 0 ? -1 : 1;
            player.selectedSlot = (player.selectedSlot + dir + 9) % 9;
            scrollOffset = 0.0;
        }
        for (int i = 0; i < 9; ++i) {
            if (glfwGetKey(renderer->window, GLFW_KEY_1 + i) == GLFW_PRESS) {
                player.selectedSlot = i;
            }
        }

        static float accumulator = 0;
        accumulator += dt;
        while (accumulator >= 0.05f) {
            player.update(0.05f, *world);
            world->updatePlayerPosition((int)player.position.x, (int)player.position.z, settings.renderDistance);
            accumulator -= 0.05f;
        }

        static std::vector<World::PendingUpload> pendingUploads;
        if (pendingUploads.empty())
            pendingUploads = world->drainUploads();

        if (pendingUploads.size() > 1) {
            int px = (int)player.position.x;
            int pz = (int)player.position.z;
            std::sort(pendingUploads.begin(), pendingUploads.end(),
                [px, pz](const World::PendingUpload& a, const World::PendingUpload& b) {
                    int ax = a.cx * 16 + 8, az = a.cz * 16 + 8;
                    int bx = b.cx * 16 + 8, bz = b.cz * 16 + 8;
                    int da = (ax - px) * (ax - px) + (az - pz) * (az - pz);
                    int db = (bx - px) * (bx - px) + (bz - pz) * (bz - pz);
                    return da < db;
                });
        }

        // OPT 3: adaptive upload budget
        int budget = frameTimeAvg < 0.011 ? 8
                   : frameTimeAvg < 0.016 ? 4
                   : frameTimeAvg < 0.025 ? 2
                   : 1;
        int uploadBudget = std::min(budget, (int)pendingUploads.size());
        for (int i = 0; i < uploadBudget; ++i) {
            renderer->uploadChunkMesh(pendingUploads[i].cx, pendingUploads[i].cz, pendingUploads[i].mesh);
        }
        pendingUploads.erase(pendingUploads.begin(), pendingUploads.begin() + uploadBudget);

        if (glfwGetKey(renderer->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(renderer->window, true);

        static bool leftPressed = false, rightPressed = false;
        if (glfwGetMouseButton(renderer->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !leftPressed) {
            player.breakBlock(*world); leftPressed = true;
        } else if (glfwGetMouseButton(renderer->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) leftPressed = false;
        if (glfwGetMouseButton(renderer->window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !rightPressed) {
            player.placeBlock(*world); rightPressed = true;
        } else if (glfwGetMouseButton(renderer->window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) rightPressed = false;

        IVec3 targetBlock;
        bool hasTarget = player.getTargetBlock(*world, targetBlock);
        glm::mat4 proj = glm::perspective(glm::radians(settings.fov), 1280.0f / 720.0f, 0.1f, 1000.0f);
        glm::mat4 view = player.getViewMatrix();

        renderer->m_camPos = player.position + Vec3(0, 1.6f, 0);
        renderer->beginFrame(proj * view);
        renderer->renderChunks(view, proj);
        if (hasTarget) renderer->renderBlockHighlight(targetBlock);
        renderer->renderCrosshair();
        renderer->renderHotbar(player.selectedSlot);

        ++fpsFrames;
        if (now - fpsLast >= 0.25) {
            fpsStr = "FPS: " + std::to_string((int)std::round(fpsFrames / (now - fpsLast)));
            fpsFrames = 0;
            fpsLast = now;
        }
        renderer->renderText(fpsStr, -0.98f, 0.92f, 0.06f);

        renderer->endFrame();
        glfwPollEvents();
    }
}
