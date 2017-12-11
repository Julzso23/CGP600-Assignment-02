#pragma once

// #define _XM_NO_INTRINSICS_
// #define XM_NO_ALIGNMENT
#include <DirectXMath.h>

struct Vertex
{
    DirectX::XMFLOAT4 position;
    DirectX::XMFLOAT4 colour;
    DirectX::XMFLOAT2 textureCoord;
	DirectX::XMFLOAT4 normal;
    DirectX::XMFLOAT4 tangent;
    DirectX::XMFLOAT4 binormal;
};
