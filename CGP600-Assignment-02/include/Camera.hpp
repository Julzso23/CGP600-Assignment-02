#pragma once

#include <d3d11.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
    private:
        float fieldOfView;
        float aspectRatio;
        float nearClippingPlane, farClippingPlane;

        XMVECTOR position, rotation;

        XMMATRIX projection, world;
    public:
        XMMATRIX getProjectionMatrix() const;
        XMMATRIX getWorldMatrix() const;

        void setFieldOfView(float value);
        float getFieldOfView() const;

        void setAspectRatio(UINT width, UINT height);
        float getAspectRatio() const;

        void setClippingPlanes(float nearClippingPlane, float farClippingPlane);
        float getNearClippingPlane() const;
        float getFarClippingPlane() const;

        void setPosition(XMVECTOR position);
        XMVECTOR getPosition() const;

        void setRotation(XMVECTOR rotation);
        XMVECTOR getRotation() const;
};
