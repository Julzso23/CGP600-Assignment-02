#pragma once

#include <vector>

class PerlinNoise
{
    private:
        std::vector<int> permutation;

        float fade(float time);
        float lerp(float time, float a, float b);
        float gradient(int hash, float x, float y, float z);
    public:
        PerlinNoise(unsigned int seed);
        float noise(float x, float y, float z);
        std::vector<int> getPermutation();
};
