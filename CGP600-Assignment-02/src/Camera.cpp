#include "Camera.hpp"

XMMATRIX Camera::getViewMatrix() const
{
    std::lock_guard<std::mutex> lock(mutex);

    XMMATRIX view = XMMatrixRotationZ(XMConvertToRadians(rotation.vector4_f32[2]));
    view *= XMMatrixRotationX(XMConvertToRadians(rotation.vector4_f32[0]));
    view *= XMMatrixRotationY(XMConvertToRadians(rotation.vector4_f32[1]));
    view *= XMMatrixTranslation(position.vector4_f32[0], position.vector4_f32[1], position.vector4_f32[2]);
    view = XMMatrixInverse(NULL, view);
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

void Camera::setPosition(XMVECTOR position)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->position = position;
}

XMVECTOR Camera::getPosition() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return position;
}

void Camera::setRotation(XMVECTOR rotation)
{
    std::lock_guard<std::mutex> lock(mutex);

    this->rotation = rotation;
}

XMVECTOR Camera::getRotation() const
{
    std::lock_guard<std::mutex> lock(mutex);

    return rotation;
}
