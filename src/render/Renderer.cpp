#include "Renderer.h"
#include "../core/Block.h"
#include <iostream>
#include <vector>

static const char* vertexSrc = R"(
#version 460 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aTexCoord;
layout(location=2) in float aTexLayer;
layout(location=3) in float aAO;
uniform mat4 uMVP;
out vec2 vTexCoord;
out float vTexLayer;
out float vAO;
void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
    vTexCoord = aTexCoord;
    vTexLayer = aTexLayer;
    vAO = aAO;
}
)";

static const char* fragmentSrc = R"(
#version 460 core
in vec2 vTexCoord;
in float vTexLayer;
in float vAO;
out vec4 FragColor;
uniform sampler2DArray uTexArray;
void main() {
    vec4 col = texture(uTexArray, vec3(vTexCoord, vTexLayer));
    FragColor = vec4(col.rgb * vAO, col.a);
}
)";

static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    int ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, 1024, nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
    }
    return s;
}

Renderer::Renderer(int width, int height) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(width, height, "Minecraft Clone", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGL((GLADloadfunc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    int ok;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(program, 1024, nullptr, log);
        std::cerr << "Program link error: " << log << std::endl;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &textureArray);
    int numBlocks = (int)Block::ID::COUNT;
    glTextureStorage3D(textureArray, 1, GL_RGBA8, 16, 16, numBlocks);

    std::vector<uint8_t> tex(16 * 16 * 4);
    for (int i = 0; i < numBlocks; ++i) {
        uint8_t r = (uint8_t)((i * 70) % 255);
        uint8_t g = (uint8_t)((i * 130) % 255);
        uint8_t b = (uint8_t)((i * 190) % 255);
        for (int p = 0; p < 16 * 16; ++p) {
            tex[p * 4 + 0] = r;
            tex[p * 4 + 1] = g;
            tex[p * 4 + 2] = b;
            tex[p * 4 + 3] = 255;
        }
        glTextureSubImage3D(textureArray, 0, 0, 0, i, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE, tex.data());
    }
    glTextureParameteri(textureArray, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(textureArray, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

Renderer::~Renderer() {
    for (auto& [k, cgl] : chunkMeshes) {
        glDeleteVertexArrays(1, &cgl.vao);
        glDeleteBuffers(1, &cgl.vbo);
        glDeleteBuffers(1, &cgl.ebo);
    }
    glDeleteTextures(1, &textureArray);
    glDeleteProgram(program);
    glfwTerminate();
}

void Renderer::beginFrame(const glm::mat4& projView) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, "uMVP"), 1, GL_FALSE, &projView[0][0]);
    glBindTextureUnit(0, textureArray);
}

void Renderer::uploadChunkMesh(int cx, int cz, const ChunkMesh& mesh) {
    int64_t key = (int64_t(cx) << 32) | uint32_t(cz);
    auto& cgl = chunkMeshes[key];
    if (cgl.vao) {
        glDeleteVertexArrays(1, &cgl.vao);
        glDeleteBuffers(1, &cgl.vbo);
        glDeleteBuffers(1, &cgl.ebo);
    }
    glCreateVertexArrays(1, &cgl.vao);
    glCreateBuffers(1, &cgl.vbo);
    glCreateBuffers(1, &cgl.ebo);
    glNamedBufferData(cgl.vbo, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);
    glNamedBufferData(cgl.ebo, mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), GL_STATIC_DRAW);

    glEnableVertexArrayAttrib(cgl.vao, 0);
    glVertexArrayAttribFormat(cgl.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(cgl.vao, 0, 0);

    glEnableVertexArrayAttrib(cgl.vao, 1);
    glVertexArrayAttribFormat(cgl.vao, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glVertexArrayAttribBinding(cgl.vao, 1, 0);

    glEnableVertexArrayAttrib(cgl.vao, 2);
    glVertexArrayAttribFormat(cgl.vao, 2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float));
    glVertexArrayAttribBinding(cgl.vao, 2, 0);

    glEnableVertexArrayAttrib(cgl.vao, 3);
    glVertexArrayAttribFormat(cgl.vao, 3, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float));
    glVertexArrayAttribBinding(cgl.vao, 3, 0);

    glVertexArrayVertexBuffer(cgl.vao, 0, cgl.vbo, 0, 8 * sizeof(float));
    glVertexArrayElementBuffer(cgl.vao, cgl.ebo);
    cgl.indexCount = (int)mesh.indices.size();
}

void Renderer::renderChunks(const glm::mat4& view, const glm::mat4& proj) {
    for (auto& [key, cgl] : chunkMeshes) {
        glBindVertexArray(cgl.vao);
        glDrawElements(GL_TRIANGLES, cgl.indexCount, GL_UNSIGNED_INT, nullptr);
    }
}

void Renderer::endFrame() {
    glfwSwapBuffers(window);
}
