#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

class PerlinNoiseCompute
{
    private:
        ID3D11Device* device;
        ID3D11DeviceContext* immediateContext;
        ID3D11ComputeShader* shader = nullptr;
        ID3D11Buffer* dataBuffer = nullptr;
        ID3D11Buffer* permutationBuffer = nullptr;
        std::vector<uint8_t> blockValues;
        std::vector<UINT> permutation;

        void generatePermutation(unsigned int seed);
    public:
        ~PerlinNoiseCompute();
        void initialise(ID3D11Device* device, ID3D11DeviceContext* immediateContext, unsigned int seed);
        void run();
        std::vector<bool> getBlockValues();
};
