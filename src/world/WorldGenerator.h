#pragma once
#include "Chunk.h"
#include "Noise.h"

class WorldGenerator {
public:
    WorldGenerator(uint64_t seed);
    void generate(Chunk& chunk);
private:
    PerlinNoise continentalness, erosion;
    double sampleHeight(int wx, int wz) const;
};
