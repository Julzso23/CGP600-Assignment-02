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
        static DirectX::XMVECTOR vectorConvertToRadians(DirectX::XMVECTOR vector);
        static DirectX::XMVECTOR vectorConvertToDegrees(DirectX::XMVECTOR vector);

        Transformable();

        void setPosition(DirectX::XMVECTOR position);
        DirectX::XMVECTOR getPosition() const;

        void setRotation(DirectX::XMVECTOR rotation);
        DirectX::XMVECTOR getRotation() const;
};
