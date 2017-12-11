#pragma once

// #define _XM_NO_INTRINSICS_
// #define XM_NO_ALIGNMENT
#include <DirectXMath.h>

struct VertexConstantBuffer
{
    DirectX::XMMATRIX worldViewProjection;
    DirectX::XMFLOAT4 ambientLightColour;
};

struct PixelConstantBuffer
{
    DirectX::XMVECTOR lightDirection;
    DirectX::XMFLOAT4 directionalLightColour;
    DirectX::XMVECTOR pointLightPosition;
    DirectX::XMFLOAT4 pointLightColour;
    float pointLightFalloff;
    DirectX::XMFLOAT3 padding;
};
