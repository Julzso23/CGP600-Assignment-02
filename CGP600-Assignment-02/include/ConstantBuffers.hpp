#pragma once

#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <DirectXMath.h>

struct ConstantBuffer0
{
    DirectX::XMMATRIX worldViewProjection;
    DirectX::XMVECTOR lightDirection;
    DirectX::XMVECTOR directionalLightColour;
    DirectX::XMVECTOR ambientLightColour;
};
