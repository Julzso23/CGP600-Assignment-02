#include "collision/CollisionCompute.hpp"
#include <d3dcompiler.h>

void CollisionCompute::initialise(ID3D11Device* device, ID3D11DeviceContext* immediateContext)
{
    this->device = device;
    this->immediateContext = immediateContext;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT result = D3DCompileFromFile(
        L"shaders/collisionCompute.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "CShader", "cs_5_0", flags, 0, &shaderBlob, &errorBlob
    );

    if (errorBlob != 0)
    {
        OutputDebugString((char*)errorBlob->GetBufferPointer());
        errorBlob->Release();
        if (FAILED(result))
        {
            OutputDebugString("#### Failed to compile compute shader! ####\n");
            //return result;
        }
    }

    result = device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &shader);

    if (FAILED(result))
    {
        OutputDebugString("#### Failed to create compute shader! ####\n");
    }
}

void CollisionCompute::run()
{
    D3D11_BUFFER_DESC gpuBufferDescription;
    ZeroMemory(&gpuBufferDescription, sizeof(gpuBufferDescription));
    gpuBufferDescription.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    gpuBufferDescription.ByteWidth = sizeof(ComputeBlock) * (UINT)blocks.size();
    gpuBufferDescription.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    gpuBufferDescription.StructureByteStride = sizeof(ComputeBlock);

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = blocks.data();
    
    device->CreateBuffer(&gpuBufferDescription, &initData, &dataBuffer);

    immediateContext->CSSetShader(shader, NULL, 0);
    immediateContext->Dispatch(64, 64, 64);
}
