#include "DirectionalLight.hpp"

DirectX::XMVECTOR DirectionalLight::ambientColour;

void DirectionalLight::setAmbientColour(DirectX::XMVECTOR colour)
{
    ambientColour = colour;
}

DirectX::XMVECTOR DirectionalLight::getAmbientColour()
{
    return ambientColour;
}

void DirectionalLight::setColour(DirectX::XMVECTOR colour)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->colour = colour;
}

DirectX::XMVECTOR DirectionalLight::getColour() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return colour;
}

void DirectionalLight::setDirection(DirectX::XMVECTOR direction)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->direction = direction;
}

DirectX::XMVECTOR DirectionalLight::getDirection() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return direction;
}
