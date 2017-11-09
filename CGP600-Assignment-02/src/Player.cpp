#include "Player.hpp"
#include <iostream>

Player::Player(HWND* windowHandle) :
    window(windowHandle)
{
    cameraOffset = XMVectorSet(0.f, 0.5f, 0.f, 0.f);

    RECT rect;
    GetClientRect(*window, &rect);
    UINT width = rect.right - rect.left;
    UINT height = rect.bottom - rect.top;

    camera.setFieldOfView(60.f);
    camera.setClippingPlanes(0.1f, 1000.f);
    camera.setPosition(position + cameraOffset);
    camera.setAspectRatio(width, height);
}

Camera* Player::getCamera()
{
    return &camera;
}

void Player::setPosition(XMVECTOR position)
{
    Transformable::setPosition(position);
    camera.setPosition(position + cameraOffset);
}

void Player::update(float deltaTime)
{
    if (GetForegroundWindow() != *window) return;

    XMVECTOR positionOffset = XMVectorZero();
    if (GetKeyState('W') & 0x8000)
    {
        positionOffset = XMVectorSetZ(positionOffset, XMVectorGetZ(positionOffset) + moveSpeed * deltaTime);
    }
    if (GetKeyState('S') & 0x8000)
    {
        positionOffset = XMVectorSetZ(positionOffset, XMVectorGetZ(positionOffset) - moveSpeed * deltaTime);
    }
    if (GetKeyState('A') & 0x8000)
    {
        positionOffset = XMVectorSetX(positionOffset, XMVectorGetX(positionOffset) - moveSpeed * deltaTime);
    }
    if (GetKeyState('D') & 0x8000)
    {
        positionOffset = XMVectorSetX(positionOffset, XMVectorGetX(positionOffset) + moveSpeed * deltaTime);
    }
    if (GetKeyState(VK_SPACE) & 0x8000)
    {
        positionOffset = XMVectorSetY(positionOffset, XMVectorGetY(positionOffset) + moveSpeed * deltaTime);
    }
    if (GetKeyState(VK_LCONTROL) & 0x8000)
    {
        positionOffset = XMVectorSetY(positionOffset, XMVectorGetY(positionOffset) - moveSpeed * deltaTime);
    }

    float cameraAngle = XMVectorGetY(camera.getRotation());
    positionOffset = XMVector3Transform(positionOffset, XMMatrixRotationY(XMConvertToRadians(cameraAngle)));

    setPosition(getPosition() + positionOffset);

    RECT rect;
    GetClientRect(*window, &rect);
    UINT width = rect.right - rect.left;
    UINT height = rect.bottom - rect.top;

    POINT mousePosition;
    GetCursorPos(&mousePosition);
    ScreenToClient(*window, &mousePosition);

    XMVECTOR rotation = camera.getRotation();
    rotation = XMVectorSetY(rotation, XMVectorGetY(rotation) + (mousePosition.x - (int)(width / 2)) / 10.f);
    rotation = XMVectorSetX(rotation, XMVectorGetX(rotation) + (mousePosition.y - (int)(height / 2)) / 10.f);
    camera.setRotation(rotation);
}

void Player::setCameraAspectRatio(UINT width, UINT height)
{
    camera.setAspectRatio(width, height);
}
