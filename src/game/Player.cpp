#include "Player.h"
#include "../world/World.h"
#include "../core/Block.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

void Player::update(float dt, World& world) {
    if (!flying) velocity.y -= 20.0f * dt;
    velocity.y = glm::clamp(velocity.y, -30.0f, 30.0f);
    Vec3 delta = velocity * dt;

    auto collideAxis = [&](float& posComp, float deltaComp, float halfSize, int axis, int yOff) {
        float target = posComp + deltaComp;
        int yBase = (int)floor(position.y);
        if (deltaComp > 0) {
            int edge = (int)floor(target + halfSize);
            if (world.getBlock(edge, yBase + yOff, (int)floor(position.z)) != (uint16_t)Block::ID::air) {
                posComp = (float)(edge - halfSize - 0.001);
                velocity[axis] = 0;
                return;
            }
        } else if (deltaComp < 0) {
            int edge = (int)floor(target - halfSize);
            if (world.getBlock(edge, yBase + yOff, (int)floor(position.z)) != (uint16_t)Block::ID::air) {
                posComp = (float)(edge + halfSize + 0.001);
                velocity[axis] = 0;
                return;
            }
        }
        posComp = target;
    };

    collideAxis(position.x, delta.x, 0.3f, 0, (int)floor(position.y));
    collideAxis(position.y, delta.y, 0.0f, 1, 0);
    collideAxis(position.z, delta.z, 0.3f, 2, (int)floor(position.y));

    onGround = (velocity.y == 0);
    velocity.x *= 0.9f;
    velocity.z *= 0.9f;
}

void Player::processInput(GLFWwindow* win, float dt) {
    float speed = flying ? 15.0f : 5.0f;
    Vec3 front(cos(glm::radians(yaw)), 0, sin(glm::radians(yaw)));
    Vec3 right = glm::cross(front, Vec3(0, 1, 0));
    Vec3 move(0);
    if (glfwGetKey(win, GLFW_KEY_W)) move += front;
    if (glfwGetKey(win, GLFW_KEY_S)) move -= front;
    if (glfwGetKey(win, GLFW_KEY_A)) move -= right;
    if (glfwGetKey(win, GLFW_KEY_D)) move += right;
    if (glm::length(move) > 0) move = glm::normalize(move);
    velocity.x = move.x * speed;
    velocity.z = move.z * speed;
    if (flying) {
        if (glfwGetKey(win, GLFW_KEY_SPACE)) velocity.y = speed;
        if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT)) velocity.y = -speed;
    } else {
        if (glfwGetKey(win, GLFW_KEY_SPACE) && onGround) velocity.y = 8.0f;
    }

    double mx, my;
    glfwGetCursorPos(win, &mx, &my);
    static double lastX = mx, lastY = my;
    float dx = float(mx - lastX) * 0.1f;
    float dy = float(lastY - my) * 0.1f;
    lastX = mx;
    lastY = my;
    yaw += dx;
    pitch += dy;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
}

void Player::breakBlock(World& world) {
    Vec3 dir(cos(glm::radians(pitch)) * cos(glm::radians(yaw)),
             sin(glm::radians(pitch)),
             cos(glm::radians(pitch)) * sin(glm::radians(yaw)));
    Vec3 eye = position + Vec3(0, 1.6f, 0);
    for (float i = 0; i < 8.0f; i += 0.1f) {
        IVec3 bp((int)floor(eye.x + dir.x * i),
                 (int)floor(eye.y + dir.y * i),
                 (int)floor(eye.z + dir.z * i));
        if (world.getBlock(bp.x, bp.y, bp.z) != (uint16_t)Block::ID::air) {
            world.setBlock(bp.x, bp.y, bp.z, (uint16_t)Block::ID::air);
            return;
        }
    }
}

bool Player::getTargetBlock(World& world, IVec3& outPos) const {
    Vec3 dir(cos(glm::radians(pitch)) * cos(glm::radians(yaw)),
             sin(glm::radians(pitch)),
             cos(glm::radians(pitch)) * sin(glm::radians(yaw)));
    Vec3 eye = position + Vec3(0, 1.6f, 0);
    for (float i = 0; i < 8.0f; i += 0.1f) {
        IVec3 bp((int)floor(eye.x + dir.x * i),
                 (int)floor(eye.y + dir.y * i),
                 (int)floor(eye.z + dir.z * i));
        if (world.getBlock(bp.x, bp.y, bp.z) != (uint16_t)Block::ID::air) {
            outPos = bp;
            return true;
        }
    }
    return false;
}

void Player::placeBlock(World& world) {
    Vec3 dir(cos(glm::radians(pitch)) * cos(glm::radians(yaw)),
             sin(glm::radians(pitch)),
             cos(glm::radians(pitch)) * sin(glm::radians(yaw)));
    Vec3 eye = position + Vec3(0, 1.6f, 0);
    for (float i = 0; i < 8.0f; i += 0.1f) {
        IVec3 bp((int)floor(eye.x + dir.x * i),
                 (int)floor(eye.y + dir.y * i),
                 (int)floor(eye.z + dir.z * i));
        if (world.getBlock(bp.x, bp.y, bp.z) != (uint16_t)Block::ID::air) {
            IVec3 prev((int)floor(eye.x + dir.x * (i - 0.2f)),
                       (int)floor(eye.y + dir.y * (i - 0.2f)),
                       (int)floor(eye.z + dir.z * (i - 0.2f)));
            world.setBlock(prev.x, prev.y, prev.z, (uint16_t)Block::ID::cobblestone);
            return;
        }
    }
}
