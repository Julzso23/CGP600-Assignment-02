#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "PerlinNoise.hpp"

class PerlinNoiseCompute
{
    private:
        ID3D11Device* device;
        ID3D11DeviceContext* immediateContext;
        ID3D11ComputeShader* shader = nullptr;
        ID3D11Buffer* dataBuffer = nullptr;
        ID3D11Buffer* permutationBuffer = nullptr;
        std::vector<UINT> blockValues;
        std::vector<int> permutation;
        PerlinNoise perlinNoise;
    public:
        PerlinNoiseCompute();
        ~PerlinNoiseCompute();
        void initialise(ID3D11Device* device, ID3D11DeviceContext* immediateContext);
        void run();
};
