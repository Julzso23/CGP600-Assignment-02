#include "Light.hpp"

void Light::setColour(DirectX::XMVECTOR colour)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->colour = colour;
}

DirectX::XMVECTOR Light::getColour() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return colour;
}

void Light::setDirection(DirectX::XMVECTOR direction)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->direction = direction;
}

DirectX::XMVECTOR Light::getDirection() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return direction;
}
