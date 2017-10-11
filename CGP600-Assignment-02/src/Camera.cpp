#include "Camera.hpp"

XMMATRIX Camera::getProjectionMatrix() const
{
    return XMMatrixPerspectiveFovLH(XMConvertToRadians(fieldOfView), aspectRatio, nearClippingPlane, farClippingPlane);
}

XMMATRIX Camera::getWorldMatrix() const
{
    XMMATRIX world = XMMatrixRotationZ(XMConvertToRadians(rotation.vector4_f32[2]));
    world *= XMMatrixRotationX(XMConvertToRadians(rotation.vector4_f32[0]));
    world *= XMMatrixRotationY(XMConvertToRadians(rotation.vector4_f32[1]));
    world *= XMMatrixTranslation(position.vector4_f32[0], position.vector4_f32[1], position.vector4_f32[2]);

    return XMMatrixInverse(NULL, world);
}

void Camera::setFieldOfView(float value)
{
    fieldOfView = value;
}

float Camera::getFieldOfView() const
{
    return fieldOfView;
}

void Camera::setAspectRatio(UINT width, UINT height)
{
    aspectRatio = (float)width / (float)height;
}

float Camera::getAspectRatio() const
{
    return aspectRatio;
}

void Camera::setClippingPlanes(float nearClippingPlane, float farClippingPlane)
{
    this->nearClippingPlane = nearClippingPlane;
    this->farClippingPlane = farClippingPlane;
}

float Camera::getNearClippingPlane() const
{
    return nearClippingPlane;
}

float Camera::getFarClippingPlane() const
{
    return farClippingPlane;
}

void Camera::setPosition(XMVECTOR position)
{
    this->position = position;
}

XMVECTOR Camera::getPosition() const
{
    return position;
}

void Camera::setRotation(XMVECTOR rotation)
{
    this->rotation = rotation;
}

XMVECTOR Camera::getRotation() const
{
    return rotation;
}
