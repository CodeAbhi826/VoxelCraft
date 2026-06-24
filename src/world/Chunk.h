#pragma once
#include <cstdint>
#include <array>
#include <atomic>
#include "ChunkSection.h"

struct Chunk {
    int32_t x, z;
    static constexpr int kSections = 24;
    ChunkSection sections[kSections];
    std::array<int16_t, 256> heightmap;
    enum State { Empty, Generating, Lit, Meshing, Ready, Unloading };
    std::atomic<State> state = Empty;

    Chunk(int cx, int cz) : x(cx), z(cz) {}
    BlockStateID getBlock(int bx, int by, int bz) const {
        if (by < -64 || by >= 320) return (uint16_t)Block::ID::air;
        int sec = (by + 64) / 16;
        int ly = (by + 64) % 16;
        return sections[sec].getBlock(bx, ly, bz);
    }
    void setBlock(int bx, int by, int bz, BlockStateID s) {
        if (by < -64 || by >= 320) return;
        int sec = (by + 64) / 16;
        int ly = (by + 64) % 16;
        sections[sec].setBlock(bx, ly, bz, s);
    }
};
