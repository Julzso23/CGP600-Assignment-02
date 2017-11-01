#include "Camera.hpp"

XMMATRIX Camera::getViewMatrix() const
{
    std::lock_guard<std::mutex> lock(mutex);

    XMMATRIX view = XMMatrixRotationRollPitchYawFromVector(Transformable::vectorConvertToRadians(rotation));
    view *= XMMatrixTranslationFromVector(position);
    view = XMMatrixInverse(nullptr, view);
    view *= XMMatrixPerspectiveFovLH(XMConvertToRadians(fieldOfView), aspectRatio, nearClippingPlane, farClippingPlane);

    return view;
}

void Camera::setFieldOfView(float value)
{
    std::lock_guard<std::mutex> lock(mutex);

    fieldOfView = value;
}

float Camera::getFieldOfView() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return fieldOfView;
}

void Camera::setAspectRatio(UINT width, UINT height)
{
    std::lock_guard<std::mutex> lock(mutex);

    aspectRatio = (float)width / (float)height;
}

float Camera::getAspectRatio() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return aspectRatio;
}

void Camera::setClippingPlanes(float nearClippingPlane, float farClippingPlane)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->nearClippingPlane = nearClippingPlane;
    this->farClippingPlane = farClippingPlane;
}

float Camera::getNearClippingPlane() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return nearClippingPlane;
}

float Camera::getFarClippingPlane() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return farClippingPlane;
}
