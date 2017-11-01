#pragma once

#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <DirectXMath.h>
#include <mutex>

class Transformable
{
    private:
        mutable std::mutex mutex;
    protected:
        DirectX::XMVECTOR position, rotation;
    public:
        void setPosition(DirectX::XMVECTOR position);
        DirectX::XMVECTOR getPosition() const;

        void setRotation(DirectX::XMVECTOR rotation);
        DirectX::XMVECTOR getRotation() const;
};
