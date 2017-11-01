#pragma once

#include <d3d11.h>
#include <mutex>
#include "Transformable.hpp"

using namespace DirectX;

class Camera : public Transformable
{
    private:
        float fieldOfView;
        float aspectRatio;
        float nearClippingPlane, farClippingPlane;

        mutable std::mutex mutex;
    public:
        XMMATRIX getViewMatrix() const;

        void setFieldOfView(float value);
        float getFieldOfView() const;

        void setAspectRatio(UINT width, UINT height);
        float getAspectRatio() const;

        void setClippingPlanes(float nearClippingPlane, float farClippingPlane);
        float getNearClippingPlane() const;
        float getFarClippingPlane() const;
};
