#include "WorldGenerator.h"

WorldGenerator::WorldGenerator(uint64_t seed)
    : continentalness(seed), erosion(seed + 1) {}

double WorldGenerator::sampleHeight(int wx, int wz) const {
    double e = continentalness.octave2D(wx * 0.005, wz * 0.005, 5);
    double p = erosion.octave2D(wx * 0.007, wz * 0.007, 4);
    return 64.0 + e * 20.0 + p * 10.0;
}

void WorldGenerator::generate(Chunk& chunk) {
    int ox = chunk.x * 16, oz = chunk.z * 16;
    for (int x = 0; x < 16; ++x) {
        for (int z = 0; z < 16; ++z) {
            int wx = ox + x, wz = oz + z;
            double h = sampleHeight(wx, wz);
            int height = (int)h;
            chunk.heightmap[x + z * 16] = height;
            for (int y = -64; y < height - 3; ++y)
                chunk.setBlock(x, y, z, (uint16_t)Block::ID::stone);
            for (int y = height - 3; y < height; ++y)
                chunk.setBlock(x, y, z, (uint16_t)Block::ID::dirt);
            chunk.setBlock(x, height, z, (uint16_t)Block::ID::grass_block);
            chunk.setBlock(x, -64, z, (uint16_t)Block::ID::bedrock);
        }
    }
    chunk.state = Chunk::Lit;
}
