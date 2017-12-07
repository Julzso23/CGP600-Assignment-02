#include "PointLight.hpp"

void PointLight::setColour(DirectX::XMFLOAT4 colour)
{
    this->colour = colour;
}

DirectX::XMFLOAT4 PointLight::getColour() const
{
    return colour;
}
