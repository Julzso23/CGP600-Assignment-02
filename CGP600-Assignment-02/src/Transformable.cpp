#include "Transformable.hpp"

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
