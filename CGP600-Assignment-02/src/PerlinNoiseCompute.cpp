#include "PerlinNoiseCompute.hpp"
#include <d3dcompiler.h>
#include <random>
#include <numeric>

void PerlinNoiseCompute::generatePermutation(unsigned int seed)
{
    permutation.resize(256);
    std::iota(permutation.begin(), permutation.end(), 0);
    std::default_random_engine engine(seed);
    std::shuffle(permutation.begin(), permutation.end(), engine);
    permutation.insert(permutation.end(), permutation.begin(), permutation.end());
}

PerlinNoiseCompute::~PerlinNoiseCompute()
{
    if (dataBuffer) dataBuffer->Release();
    if (permutationBuffer) permutationBuffer->Release();
}

void PerlinNoiseCompute::initialise(ID3D11Device* device, ID3D11DeviceContext* immediateContext, unsigned int seed)
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
        L"shaders/perlinNoiseCompute.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
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

    blockValues.assign(64 * 64 * 64, 0);

    generatePermutation(seed);
}

void PerlinNoiseCompute::run()
{
    D3D11_BUFFER_DESC bufferDescription;
    ZeroMemory(&bufferDescription, sizeof(bufferDescription));
    bufferDescription.Usage = D3D11_USAGE_DEFAULT;
    bufferDescription.ByteWidth = sizeof(uint8_t) * (UINT)blockValues.size();
    bufferDescription.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
    bufferDescription.StructureByteStride = sizeof(UINT);

    // Create data buffer
    HRESULT result = device->CreateBuffer(&bufferDescription, NULL, &dataBuffer);

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    immediateContext->Map(dataBuffer, NULL, D3D11_MAP_WRITE, NULL, &mappedSubresource);
    memcpy(mappedSubresource.pData, blockValues.data(), (UINT)blockValues.size() * sizeof(uint8_t));
    immediateContext->Unmap(dataBuffer, NULL);

    bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
    bufferDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDescription.ByteWidth = sizeof(UINT) * (UINT)permutation.size();
    bufferDescription.StructureByteStride = sizeof(UINT);

    // Create permutation buffer
    result = device->CreateBuffer(&bufferDescription, NULL, &permutationBuffer);

    immediateContext->Map(permutationBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
    memcpy(mappedSubresource.pData, permutation.data(), (UINT)permutation.size() * sizeof(UINT));
    immediateContext->Unmap(permutationBuffer, NULL);

    ID3D11UnorderedAccessView* dataUAV = nullptr;

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDescription;
    ZeroMemory(&uavDescription, sizeof(uavDescription));
    uavDescription.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDescription.Buffer.NumElements = (UINT)blockValues.size();
    uavDescription.Format = DXGI_FORMAT_R8_UINT;
    uavDescription.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

    result = device->CreateUnorderedAccessView(dataBuffer, &uavDescription, &dataUAV);

    ID3D11ShaderResourceView* permutationResource = nullptr;

    D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription;
    ZeroMemory(&resourceViewDescription, sizeof(resourceViewDescription));
    resourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    resourceViewDescription.BufferEx.NumElements = (UINT)permutation.size();
    resourceViewDescription.Format = DXGI_FORMAT_R32_UINT;

    result = device->CreateShaderResourceView(permutationBuffer, &resourceViewDescription, &permutationResource);

    immediateContext->CSSetShader(shader, NULL, 0);
    immediateContext->CSSetUnorderedAccessViews(0, 1, &dataUAV, NULL);
    immediateContext->CSSetShaderResources(0, 1, &permutationResource);
    immediateContext->Dispatch(8, 8, 8);

    immediateContext->Map(dataBuffer, NULL, D3D11_MAP_READ, NULL, &mappedSubresource);
    blockValues.assign(reinterpret_cast<uint8_t*>(mappedSubresource.pData), reinterpret_cast<uint8_t*>(mappedSubresource.pData) + (mappedSubresource.RowPitch / sizeof(uint8_t)));
    immediateContext->Unmap(dataBuffer, NULL);
}

std::vector<bool> PerlinNoiseCompute::getBlockValues()
{
    return std::vector<bool>(blockValues.begin(), blockValues.end());
}
