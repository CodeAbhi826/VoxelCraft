#include "Noise.h"
#include <cmath>
#include <algorithm>

PerlinNoise::PerlinNoise(uint32_t seed) {
    std::mt19937 rng(seed);
    for (int i = 0; i < 256; ++i) p[i] = i;
    std::shuffle(p.begin(), p.begin() + 256, rng);
    for (int i = 0; i < 256; ++i) p[i + 256] = p[i];
}
double PerlinNoise::fade(double t) const { return t * t * t * (t * (t * 6 - 15) + 10); }
double PerlinNoise::lerp(double a, double b, double t) const { return a + t * (b - a); }
double PerlinNoise::grad(int hash, double x, double y, double z) const {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}
double PerlinNoise::noise(double x, double y, double z) const {
    int X = (int)floor(x) & 255, Y = (int)floor(y) & 255, Z = (int)floor(z) & 255;
    x -= floor(x); y -= floor(y); z -= floor(z);
    double u = fade(x), v = fade(y), w = fade(z);
    int A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z;
    int B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;
    return lerp(
        lerp(lerp(grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z), u),
             lerp(grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z), u), v),
        lerp(lerp(grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1), u),
             lerp(grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1), u), v), w);
}
double PerlinNoise::noise(double x, double y) const { return noise(x, y, 0.0); }
double PerlinNoise::octave2D(double x, double y, int octaves, double persistence) const {
    double total = 0, freq = 1, amp = 1, maxVal = 0;
    for (int i = 0; i < octaves; ++i) {
        total += noise(x * freq, y * freq) * amp;
        maxVal += amp;
        amp *= persistence;
        freq *= 2;
    }
    return total / maxVal;
}
