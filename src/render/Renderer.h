#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include "MeshBuilder.h"

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();
    void beginFrame(const glm::mat4& projView);
    void uploadChunkMesh(int cx, int cz, const ChunkMesh& mesh);
    void renderChunks(const glm::mat4& view, const glm::mat4& proj);
    void endFrame();
    GLFWwindow* window;
private:
    GLuint program, textureArray;
    struct ChunkGL {
        GLuint vao = 0, vbo = 0, ebo = 0;
        int indexCount = 0;
    };
    std::unordered_map<int64_t, ChunkGL> chunkMeshes;
};
