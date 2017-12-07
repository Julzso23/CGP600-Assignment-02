#pragma once

#include "Transformable.hpp"

class PointLight : public Transformable
{
    private:
        DirectX::XMFLOAT4 colour;
        float falloff;
        mutable std::mutex mutex;
    public:
        void setColour(DirectX::XMFLOAT4 colour);
        DirectX::XMFLOAT4 getColour() const;

        void setFalloff(float value);
        float getFalloff() const;
};
