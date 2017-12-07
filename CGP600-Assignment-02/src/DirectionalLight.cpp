#include "DirectionalLight.hpp"

DirectX::XMFLOAT4 DirectionalLight::ambientColour;

void DirectionalLight::setAmbientColour(DirectX::XMFLOAT4 colour)
{
    ambientColour = colour;
}

DirectX::XMFLOAT4 DirectionalLight::getAmbientColour()
{
    return ambientColour;
}

void DirectionalLight::setColour(DirectX::XMFLOAT4 colour)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->colour = colour;
}

DirectX::XMFLOAT4 DirectionalLight::getColour() const
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
