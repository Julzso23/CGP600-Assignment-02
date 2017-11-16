#include "Player.hpp"
#include "Utility.hpp"
#include <iostream>

void Player::initialise(HWND* windowHandle)
{
    window = windowHandle;

    setSize(XMVectorSet(1.f, 2.f, 1.f, 0.f));

    cameraOffset = XMVectorSet(0.f, 0.5f, 0.f, 0.f);

    RECT rect;
    GetClientRect(*window, &rect);
    UINT width = rect.right - rect.left;
    UINT height = rect.bottom - rect.top;

    camera.setFieldOfView(60.f);
    camera.setClippingPlanes(0.1f, 1000.f);
    camera.setPosition(position + cameraOffset);
    camera.setAspectRatio(width, height);

    keyboard = std::make_unique<Keyboard>();
    mouse = std::make_unique<Mouse>();
    mouse->SetWindow(*window);
    mouse->SetMode(Mouse::MODE_RELATIVE);
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

    Keyboard::State keyState = keyboard->GetState();

    XMVECTOR positionOffset = XMVectorZero();
    if (keyState.W)
    {
        positionOffset = XMVectorSetZ(positionOffset, XMVectorGetZ(positionOffset) + 1.f);
    }
    if (keyState.S)
    {
        positionOffset = XMVectorSetZ(positionOffset, XMVectorGetZ(positionOffset) - 1.f);
    }
    if (keyState.A)
    {
        positionOffset = XMVectorSetX(positionOffset, XMVectorGetX(positionOffset) - 1.f);
    }
    if (keyState.D)
    {
        positionOffset = XMVectorSetX(positionOffset, XMVectorGetX(positionOffset) + 1.f);
    }

    positionOffset = XMVector3Normalize(positionOffset);
    positionOffset *= moveSpeed * deltaTime;

    float cameraAngle = XMVectorGetY(camera.getRotation());
    positionOffset = XMVector3Transform(positionOffset, XMMatrixRotationY(XMConvertToRadians(cameraAngle)));

    if (grounded && keyState.Space)
    {
        velocity = jumpForce;
    }
    velocity += gravity * deltaTime;
    positionOffset = XMVectorSetY(positionOffset, velocity * deltaTime);

    setPosition(getPosition() + positionOffset);

    Mouse::State mouseState = mouse->GetState();

    Segment viewRay = {
        camera.getPosition(),
        XMVector3Transform(XMVectorSet(0.f, 0.f, 3.f, 0.f), XMMatrixRotationRollPitchYawFromVector(Transformable::vectorConvertToRadians(camera.getRotation())))
    };

    if (mouseState.leftButton)
    {
        if (!leftMouseButtonDown)
        {
            leftMouseButtonDown = true;
            breakBlock(viewRay);
        }
    }
    else
    {
        leftMouseButtonDown = false;
    }

    if (mouseState.rightButton)
    {
        if (!rightMouseButtonDown)
        {
            rightMouseButtonDown = true;
            placeBlock(viewRay);
        }
    }
    else
    {
        rightMouseButtonDown = false;
    }

    XMVECTOR rotation = camera.getRotation();
    rotation = XMVectorSetY(rotation, XMVectorGetY(rotation) + (float)mouseState.x * cameraRotateSpeed);
    rotation = XMVectorSetX(rotation, Utility::clamp(XMVectorGetX(rotation) + (float)mouseState.y * cameraRotateSpeed, -90.f, 90.f));
    camera.setRotation(rotation);

    grounded = false;
}

void Player::setCameraAspectRatio(UINT width, UINT height)
{
    camera.setAspectRatio(width, height);
}

void Player::setGrounded(bool value)
{
    grounded = value;
}

void Player::setVelocity(float value)
{
	velocity = value;
}

void Player::setBreakBlockFunction(std::function<void(Segment ray)> function)
{
    breakBlock = function;
}

void Player::setPlaceBlockFunction(std::function<void(Segment ray)> function)
{
    placeBlock = function;
}
