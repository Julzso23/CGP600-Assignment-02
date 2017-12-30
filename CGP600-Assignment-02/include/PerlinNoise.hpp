#pragma once

#include <vector>
#include <Windows.h>

class PerlinNoise
{
    private:
        std::vector<UINT> permutation;

        float fade(float time);
        float lerp(float time, float a, float b);
        float gradient(int hash, float x, float y, float z);
    public:
        PerlinNoise(unsigned int seed);
        float noise(float x, float y, float z);
        std::vector<UINT> getPermutation();
};
