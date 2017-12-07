#pragma once

#include "Transformable.hpp"

class DirectionalLight
{
    private:
        static DirectX::XMFLOAT4 ambientColour;
        DirectX::XMFLOAT4 colour;
        DirectX::XMVECTOR direction;
        mutable std::mutex mutex;
    public:
        static void setAmbientColour(DirectX::XMFLOAT4 colour);
        static DirectX::XMFLOAT4 getAmbientColour();

        void setColour(DirectX::XMFLOAT4 colour);
        DirectX::XMFLOAT4 getColour() const;

        void setDirection(DirectX::XMVECTOR direction);
        DirectX::XMVECTOR getDirection() const;
};
