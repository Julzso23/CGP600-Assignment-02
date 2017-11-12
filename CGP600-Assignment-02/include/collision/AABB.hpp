#pragma once

#include "collision\Segment.hpp"
#include "collision\Sweep.hpp"
#include "Transformable.hpp"

// A class to hold axis-aligned bounding box data
class AABB : public Transformable
{
private:
    DirectX::XMVECTOR size;
public:
    AABB();

    DirectX::XMVECTOR getSize();
    void setSize(DirectX::XMVECTOR size);

    DirectX::XMVECTOR getHalf();
    DirectX::XMVECTOR getCentre();

    Hit testIntersection(DirectX::XMVECTOR point, DirectX::XMVECTOR padding = DirectX::XMVectorSet(0.f, 0.f, 0.f, 0.f));
    Hit testIntersection(Segment segment, DirectX::XMVECTOR padding = DirectX::XMVectorSet(0.f, 0.f, 0.f, 0.f));
    Hit testIntersection(AABB& other);
    Sweep sweepIntersection(AABB& other, DirectX::XMVECTOR delta);
};
