#pragma once

// #define _XM_NO_INTRINSICS_
// #define XM_NO_ALIGNMENT
#include <DirectXMath.h>
#include <mutex>
#include <vector>

class Transformable
{
    private:
        std::vector<Transformable*> children;
        DirectX::XMVECTOR localPosition = DirectX::XMVectorZero();
        mutable std::mutex mutex;
    protected:
        DirectX::XMVECTOR position, rotation;
    public:
        static DirectX::XMVECTOR vectorConvertToRadians(DirectX::XMVECTOR vector);
        static DirectX::XMVECTOR vectorConvertToDegrees(DirectX::XMVECTOR vector);

        Transformable();

        virtual void setPosition(const DirectX::XMVECTOR& position);
        DirectX::XMVECTOR getPosition() const;

        virtual void setLocalPosition(const DirectX::XMVECTOR& position);
        DirectX::XMVECTOR getLocalPosition() const;

        void setRotation(const DirectX::XMVECTOR& rotation);
        DirectX::XMVECTOR getRotation() const;

        void addChild(Transformable* child);
        void removeChild(Transformable* child);
};
