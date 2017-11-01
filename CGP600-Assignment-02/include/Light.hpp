#pragma once

#include "Transformable.hpp"

class Light
{
    private:
        DirectX::XMVECTOR colour;
        DirectX::XMVECTOR direction;
        mutable std::mutex mutex;
    public:
        void setColour(DirectX::XMVECTOR colour);
        DirectX::XMVECTOR getColour() const;

        void setDirection(DirectX::XMVECTOR direction);
        DirectX::XMVECTOR getDirection() const;
};
