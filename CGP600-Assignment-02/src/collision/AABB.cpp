#include "collision\AABB.hpp"
#include "Utility.hpp"

using namespace DirectX;

AABB::AABB() :
    size(XMVectorSet(0.f, 0.f, 0.f, 0.f))
{
}

XMVECTOR AABB::getSize()
{
    return size;
}

void AABB::setSize(XMVECTOR size)
{
    this->size = size;
}

XMVECTOR AABB::getHalf()
{
    return size / 2.f;
}

XMVECTOR AABB::getCentre()
{
    return getPosition() + getHalf();
}

// Point -> AABB collision test
Hit AABB::testIntersection(XMVECTOR point, XMVECTOR padding) // Padding pads out the size of the AABB for Minkowski difference
{
    Hit result;
    XMVECTOR difference = point - getCentre();
    XMVECTOR half = getHalf() + padding;
    XMVECTOR collisionPoint = half - XMVectorSet(abs(XMVectorGetX(difference)), abs(XMVectorGetY(difference)), abs(XMVectorGetZ(difference)), 0.f);

    result.object = this;

    if (XMVectorGetX(collisionPoint) <= 0.f || XMVectorGetY(collisionPoint) <= 0.f || XMVectorGetZ(collisionPoint) <= 0.f)
    {
        result.hit = false;
        return result;
    }

    if (XMVectorGetX(collisionPoint) < XMVectorGetY(collisionPoint) && XMVectorGetX(collisionPoint) < XMVectorGetZ(collisionPoint))
    {
        float signX = Utility::sign(XMVectorGetX(difference));

        result.delta = XMVectorSet(XMVectorGetX(collisionPoint) * signX, 0.f, 0.f, 0.f);

        result.normal = XMVectorSet(signX, 0.f, 0.f, 0.f);

        result.position = XMVectorSet(
            XMVectorGetX(getCentre()) + (XMVectorGetX(half) * signX),
            XMVectorGetY(point),
            XMVectorGetZ(point),
            0.f
        );

        result.hit = true;

        return result;
    }

    if (XMVectorGetY(collisionPoint) < XMVectorGetX(collisionPoint) && XMVectorGetY(collisionPoint) < XMVectorGetZ(collisionPoint))
    {
        float signY = Utility::sign(XMVectorGetY(difference));

        result.delta = XMVectorSet(0.f, XMVectorGetY(collisionPoint) * signY, 0.f, 0.f);

        result.normal = XMVectorSet(0.f, signY, 0.f, 0.f);

        result.position = XMVectorSet(
            XMVectorGetX(point),
            XMVectorGetY(getCentre()) + (XMVectorGetY(half) * signY),
            XMVectorGetZ(point),
            0.f
        );


        result.hit = true;

        return result;
    }

    float signZ = Utility::sign(XMVectorGetZ(difference));

    result.delta = XMVectorSet(0.f, 0.f, XMVectorGetZ(collisionPoint) * signZ, 0.f);

    result.normal = XMVectorSet(0.f, 0.f, signZ, 0.f);

    result.position = XMVectorSet(
        XMVectorGetX(point),
        XMVectorGetY(point),
        XMVectorGetZ(getCentre()) + (XMVectorGetZ(half) * signZ),
        0.f
    );

    result.hit = true;

    return result;
}

// Segment (non-infinite line) -> AABB collision test
Hit AABB::testIntersection(Segment segment, XMVECTOR padding) // Padding pads out the size of the AABB for Minkowski difference
{
    Hit result;

    XMVECTOR scale = XMVectorSet(1.f / XMVectorGetX(segment.delta), 1.f / XMVectorGetY(segment.delta), 1.f / XMVectorGetZ(segment.delta), 0.f);
    XMVECTOR sign = XMVectorSet(Utility::sign(XMVectorGetX(scale)), Utility::sign(XMVectorGetY(scale)), Utility::sign(XMVectorGetZ(scale)), 0.f);

    XMVECTOR nearTimes, farTimes;

    nearTimes = XMVectorSet(
        (XMVectorGetX(getCentre()) - XMVectorGetX(sign) * (XMVectorGetX(getHalf()) + XMVectorGetX(padding)) - XMVectorGetX(segment.position)) * XMVectorGetX(scale),
        (XMVectorGetY(getCentre()) - XMVectorGetY(sign) * (XMVectorGetY(getHalf()) + XMVectorGetY(padding)) - XMVectorGetY(segment.position)) * XMVectorGetY(scale),
        (XMVectorGetZ(getCentre()) - XMVectorGetZ(sign) * (XMVectorGetZ(getHalf()) + XMVectorGetZ(padding)) - XMVectorGetZ(segment.position)) * XMVectorGetZ(scale),
        0.f
    );

    farTimes = XMVectorSet(
        (XMVectorGetX(getCentre()) + XMVectorGetX(sign) * (XMVectorGetX(getHalf()) + XMVectorGetX(padding)) - XMVectorGetX(segment.position)) * XMVectorGetX(scale),
        (XMVectorGetY(getCentre()) + XMVectorGetY(sign) * (XMVectorGetY(getHalf()) + XMVectorGetY(padding)) - XMVectorGetY(segment.position)) * XMVectorGetY(scale),
        (XMVectorGetZ(getCentre()) + XMVectorGetZ(sign) * (XMVectorGetZ(getHalf()) + XMVectorGetZ(padding)) - XMVectorGetZ(segment.position)) * XMVectorGetZ(scale),
        0.f
    );

    result.object = this;

    if (XMVectorGetX(nearTimes) > XMVectorGetY(farTimes) || XMVectorGetX(nearTimes) > XMVectorGetZ(farTimes) ||
        XMVectorGetY(nearTimes) > XMVectorGetX(farTimes) || XMVectorGetY(nearTimes) > XMVectorGetZ(farTimes) ||
        XMVectorGetZ(nearTimes) > XMVectorGetX(farTimes) || XMVectorGetZ(nearTimes) > XMVectorGetY(farTimes))
    {
        result.hit = false;
        return result;
    }

    float nearTime = Utility::max(XMVectorGetX(nearTimes), XMVectorGetY(nearTimes), XMVectorGetZ(nearTimes));
    float farTime = Utility::min(XMVectorGetX(nearTimes), XMVectorGetY(nearTimes), XMVectorGetZ(nearTimes));

    if (nearTime >= 1.f || farTime <= 0.f)
    {
        result.hit = false;
        return result;
    }

    result.time = Utility::clamp(nearTime, 0.f, 1.f);

    if (XMVectorGetX(nearTimes) > XMVectorGetY(nearTimes) && XMVectorGetX(nearTimes) > XMVectorGetZ(nearTimes))
    {
        result.normal = XMVectorSet(-XMVectorGetX(sign), 0.f, 0.f, 0.f);
    }
    else if (XMVectorGetY(nearTimes) > XMVectorGetX(nearTimes) && XMVectorGetY(nearTimes) > XMVectorGetZ(nearTimes))
    {
        result.normal = XMVectorSet(0.f, -XMVectorGetY(sign), 0.f, 0.f);
    }
    else
    {
        result.normal = XMVectorSet(0.f, 0.f, -XMVectorGetZ(sign), 0.f);
    }

    result.delta = segment.delta * result.time;

    result.position = segment.position + result.delta;

    result.hit = true;

    return result;
}

// AABB -> AABB collision test
Hit AABB::testIntersection(AABB& other)
{
    return testIntersection(other.getCentre(), other.getHalf());
}

// AABB -> AABB sweeping collision test over distance delta
Sweep AABB::sweepIntersection(AABB& other, XMVECTOR delta)
{
    Sweep sweep;

    sweep.position = other.getCentre();
    sweep.hit = testIntersection(other);
    if (sweep.hit.hit)
    {
        sweep.time = 0.f;
        return sweep;
    }

    if (XMVectorGetX(delta) == 0.f && XMVectorGetZ(delta) == 0.f)
    {
        sweep.time = 1.f;
        return sweep;
    }

    sweep.hit = testIntersection({ other.getCentre(), delta }, other.getHalf());

    if (sweep.hit.hit)
    {
        sweep.time = Utility::clamp(sweep.hit.time - 1e-8f, 0.f, 1.f);

        sweep.position = other.getCentre() + delta * sweep.time;

        XMVECTOR direction = XMVector3Normalize(delta);

        sweep.hit.position += XMVectorSet(
            XMVectorGetX(direction) * XMVectorGetX(other.getHalf()),
            XMVectorGetY(direction) * XMVectorGetY(other.getHalf()),
            XMVectorGetZ(direction) * XMVectorGetZ(other.getHalf()),
            0.f
        );

        return sweep;
    }

    sweep.position = other.getCentre() + delta;
    sweep.time = 1.f;

    return sweep;
}
