#pragma once

// #define _XM_NO_INTRINSICS_
// #define XM_NO_ALIGNMENT
#include <DirectXMath.h>

struct Segment
{
    DirectX::XMVECTOR position;
    DirectX::XMVECTOR delta;
};