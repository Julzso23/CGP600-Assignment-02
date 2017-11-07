#include "Transformable.hpp"

using namespace DirectX;

DirectX::XMVECTOR Transformable::vectorConvertToRadians(DirectX::XMVECTOR vector)
{
    return XMVectorSet(
        XMConvertToRadians(XMVectorGetX(vector)),
        XMConvertToRadians(XMVectorGetY(vector)),
        XMConvertToRadians(XMVectorGetZ(vector)),
        0.f
    );
}

DirectX::XMVECTOR Transformable::vectorConvertToDegrees(DirectX::XMVECTOR vector)
{
    return XMVectorSet(
        XMConvertToDegrees(XMVectorGetX(vector)),
        XMConvertToDegrees(XMVectorGetY(vector)),
        XMConvertToDegrees(XMVectorGetZ(vector)),
        0.f
    );
}

Transformable::Transformable()
{
    position = XMVectorZero();
    rotation = XMVectorZero();
}

void Transformable::setPosition(DirectX::XMVECTOR position)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->position = position;
}

DirectX::XMVECTOR Transformable::getPosition() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return position;
}

void Transformable::setRotation(DirectX::XMVECTOR rotation)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->rotation = rotation;
}

DirectX::XMVECTOR Transformable::getRotation() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return rotation;
}
