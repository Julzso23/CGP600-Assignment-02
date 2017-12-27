#include "Transformable.hpp"
#include <algorithm>

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

void Transformable::setPosition(const DirectX::XMVECTOR& position)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->position = position;
    for (Transformable* child : children)
    {
        child->setPosition(position + child->getLocalPosition());
    }
}

DirectX::XMVECTOR Transformable::getPosition() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return position;
}

void Transformable::setLocalPosition(const DirectX::XMVECTOR& position)
{
    localPosition = position;
}

DirectX::XMVECTOR Transformable::getLocalPosition() const
{
    return localPosition;
}

void Transformable::setRotation(const DirectX::XMVECTOR& rotation)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->rotation = rotation;
}

DirectX::XMVECTOR Transformable::getRotation() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return rotation;
}

void Transformable::addChild(Transformable* child)
{
    children.push_back(child);
}

void Transformable::removeChild(Transformable* child)
{
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
}
