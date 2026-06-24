#pragma once
#include <glad/glad.h>
#include <cstdint>
#include <vector>

class TextureAtlas {
public:
    TextureAtlas() = default;
    ~TextureAtlas();

    void generateProcedural();
    void bind(int unit = 0) const;
    GLuint getTextureId() const { return m_textureId; }
    int getLayerCount() const { return m_layerCount; }

private:
    GLuint m_textureId = 0;
    int m_textureSize = 16;
    int m_layerCount = 0;
    bool m_loaded = false;
};
