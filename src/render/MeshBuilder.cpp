#include "MeshBuilder.h"
#include "../core/Block.h"

static const glm::ivec3 dirVectors[6] = {
    {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}
};

static const glm::vec3 faceVerts[6][4] = {
    { {0,1,0}, {1,1,0}, {1,1,1}, {0,1,1} },
    { {0,0,0}, {1,0,0}, {1,0,1}, {0,0,1} },
    { {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1} },
    { {0,0,0}, {1,0,0}, {1,1,0}, {0,1,0} },
    { {1,0,0}, {1,0,1}, {1,1,1}, {1,1,0} },
    { {0,0,0}, {0,0,1}, {0,1,1}, {0,1,0} }
};

static const glm::vec2 uvs[4] = {
    {0, 0}, {1, 0}, {1, 1}, {0, 1}
};

bool MeshBuilder::isVisible(const World& world, int wx, int wy, int wz, int dir) {
    glm::ivec3 n = glm::ivec3(wx, wy, wz) + dirVectors[dir];
    BlockStateID id = world.getBlock(n.x, n.y, n.z);
    return (id == (uint16_t)Block::ID::air || id == (uint16_t)Block::ID::water);
}

void MeshBuilder::addFace(ChunkMesh& m, const glm::vec3& pos, int dir, uint16_t texId, float ao) {
    uint32_t base = (uint32_t)(m.vertices.size() / 8);
    for (int i = 0; i < 4; ++i) {
        glm::vec3 v = pos + faceVerts[dir][i];
        m.vertices.push_back(v.x);
        m.vertices.push_back(v.y);
        m.vertices.push_back(v.z);
        m.vertices.push_back(uvs[i].x);
        m.vertices.push_back(uvs[i].y);
        m.vertices.push_back((float)texId);
        m.vertices.push_back(ao);
        m.vertices.push_back(0.0f);
    }
    m.indices.push_back(base + 0);
    m.indices.push_back(base + 1);
    m.indices.push_back(base + 2);
    m.indices.push_back(base + 2);
    m.indices.push_back(base + 3);
    m.indices.push_back(base + 0);
}

ChunkMesh MeshBuilder::build(const Chunk& chunk, const World& world) {
    ChunkMesh mesh;
    glm::ivec3 origin(chunk.x * 16, -64, chunk.z * 16);

    for (int y = 0; y < 384; ++y) {
        for (int x = 0; x < 16; ++x) {
            for (int z = 0; z < 16; ++z) {
                BlockStateID block = chunk.getBlock(x, y, z);
                if (block == (uint16_t)Block::ID::air) continue;

                int wx = origin.x + x;
                int wy = origin.y + y;
                int wz = origin.z + z;
                glm::vec3 bpos((float)(origin.x + x), (float)(origin.y + y), (float)(origin.z + z));

                uint16_t texId = block;
                if (block > (uint16_t)Block::ID::air && block < (uint16_t)Block::ID::COUNT)
                    texId = block;

                for (int d = 0; d < 6; ++d) {
                    if (isVisible(world, wx, wy, wz, d)) {
                        float ao = 0.85f;
                        if (d == 1) ao = 0.6f;
                        else if (d == 0) ao = 1.0f;
                        addFace(mesh, bpos, d, texId, ao);
                    }
                }
            }
        }
    }
    mesh.ready = true;
    return mesh;
}
