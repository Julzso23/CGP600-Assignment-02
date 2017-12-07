#include "PointLight.hpp"

void PointLight::setColour(DirectX::XMFLOAT4 colour)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->colour = colour;
}

DirectX::XMFLOAT4 PointLight::getColour() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return colour;
}

void PointLight::setFalloff(float value)
{
    std::lock_guard<std::mutex> lock(mutex);

    falloff = value;
}

float PointLight::getFalloff() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return falloff;
}
