#include "collision/CollisionCompute.hpp"
#include <d3dcompiler.h>
#include <random>

CollisionCompute::CollisionCompute() :
    perlinNoise(std::uniform_int_distribution<int>(0, 999999999)(std::random_device()))
{
}

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
    gpuBufferDescription.ByteWidth = sizeof(bool) * 64*64*64;
    gpuBufferDescription.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    gpuBufferDescription.StructureByteStride = sizeof(bool);

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = blockValues;
    
    device->CreateBuffer(&gpuBufferDescription, &initData, &dataBuffer);

    permutation = perlinNoise.getPermutation();
    gpuBufferDescription.ByteWidth = sizeof(int) * (UINT)permutation.size();
    gpuBufferDescription.StructureByteStride = sizeof(int);

    initData.pSysMem = permutation.data();

    device->CreateBuffer(&gpuBufferDescription, &initData, &permutationBuffer);

    ID3D11ShaderResourceView* resources[] = {
        nullptr,
        nullptr
    };

    D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription;
    ZeroMemory(&resourceViewDescription, sizeof(resourceViewDescription));
    resourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    resourceViewDescription.BufferEx.FirstElement = 0;

    resourceViewDescription.Format = DXGI_FORMAT_R8_UINT;
    resourceViewDescription.BufferEx.NumElements = 64 * 64 * 64;
    device->CreateShaderResourceView(dataBuffer, &resourceViewDescription, &resources[0]);

    resourceViewDescription.Format = DXGI_FORMAT_R32_SINT;
    resourceViewDescription.BufferEx.NumElements = (UINT)permutation.size();
    device->CreateShaderResourceView(permutationBuffer, &resourceViewDescription, &resources[1]);

    immediateContext->CSSetShader(shader, NULL, 0);
    immediateContext->CSSetShaderResources(0, 2, resources);
    immediateContext->Dispatch(64, 64, 64);
}
