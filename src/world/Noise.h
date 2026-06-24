#pragma once
#include <cstdint>
#include <array>
#include <random>

class PerlinNoise {
public:
    PerlinNoise(uint32_t seed);
    double noise(double x, double y) const;
    double noise(double x, double y, double z) const;
    double octave2D(double x, double y, int octaves, double persistence = 0.5) const;
private:
    std::array<int, 512> p;
    double fade(double t) const;
    double lerp(double a, double b, double t) const;
    double grad(int hash, double x, double y, double z) const;
};
