#pragma once

#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <DirectXMath.h>

class AABB;

struct Hit
{
    bool hit;
    DirectX::XMVECTOR position;
    DirectX::XMVECTOR delta;
    DirectX::XMVECTOR normal;
    float time;
    AABB* object;
};
