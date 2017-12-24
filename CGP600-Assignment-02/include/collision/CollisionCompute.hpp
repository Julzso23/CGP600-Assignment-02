#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

struct ComputeBlock
{
    bool shouldCollide;
    DirectX::XMVECTOR position;
    DirectX::XMVECTOR size;
};

class CollisionCompute
{
    private:
        ID3D11Device* device;
        ID3D11DeviceContext* immediateContext;
        ID3D11ComputeShader* shader = nullptr;
        ID3D11Buffer* dataBuffer = nullptr;
        std::vector<ComputeBlock> blocks;
    public:
        void initialise(ID3D11Device* device, ID3D11DeviceContext* immediateContext);
        void run();
};
