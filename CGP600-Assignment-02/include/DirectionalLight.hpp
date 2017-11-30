#pragma once

#include "Transformable.hpp"

class DirectionalLight
{
    private:
        static DirectX::XMVECTOR ambientColour;
        DirectX::XMVECTOR colour;
        DirectX::XMVECTOR direction;
        mutable std::mutex mutex;
    public:
        static void setAmbientColour(DirectX::XMVECTOR colour);
        static DirectX::XMVECTOR getAmbientColour();

        void setColour(DirectX::XMVECTOR colour);
        DirectX::XMVECTOR getColour() const;

        void setDirection(DirectX::XMVECTOR direction);
        DirectX::XMVECTOR getDirection() const;
};
