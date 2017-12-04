#include "Player.hpp"
#include "Utility.hpp"
#include <iostream>

void Player::initialise(HWND* windowHandle)
{
    window = windowHandle;

    cameraOffset = XMVectorSet(0.f, 0.5f, 0.f, 0.f);

    RECT rect;
    GetClientRect(*window, &rect);
    UINT width = rect.right - rect.left;
    UINT height = rect.bottom - rect.top;

    camera.setFieldOfView(fieldOfView);
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

// Update camera position when moving the player
void Player::setPosition(const XMVECTOR& position)
{
    Transformable::setPosition(position);
    camera.setPosition(position + cameraOffset);
}

void Player::update(float deltaTime)
{
    Character::update(deltaTime);

    Keyboard::State keyState = keyboard->GetState();

    // Get player movement input
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

    positionOffset = XMVector3Normalize(positionOffset); // Stop the player moving faster in a diagonal direction
    if (keyState.LeftShift)
    {
        camera.setFieldOfView(sprintFieldOfView);
        positionOffset *= moveSpeed * 2.f * deltaTime;
    }
    else
    {
        camera.setFieldOfView(fieldOfView);
        positionOffset *= moveSpeed * deltaTime;
    }

    // Move in the direction of the camera
    float cameraAngle = XMVectorGetY(camera.getRotation());
    positionOffset = XMVector3Transform(positionOffset, XMMatrixRotationY(XMConvertToRadians(cameraAngle)));

    if (keyState.Space)
    {
        jump();
    }

    positionOffset = XMVectorSetY(positionOffset, velocity * deltaTime);

    move(positionOffset); // Move the player

    Mouse::State mouseState = mouse->GetState();

    // Create a ray for raycasting when breaking/placing blocks
    Segment viewRay = {
        camera.getPosition(),
        XMVector3Transform(XMVectorSet(0.f, 0.f, reach, 0.f), XMMatrixRotationRollPitchYawFromVector(Transformable::vectorConvertToRadians(camera.getRotation())))
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

    // Rotate camera by mouse input
    XMVECTOR rotation = camera.getRotation();
    rotation = XMVectorSetY(rotation, XMVectorGetY(rotation) + (float)mouseState.x * cameraRotateSpeed);
    rotation = XMVectorSetX(rotation, Utility::clamp(XMVectorGetX(rotation) + (float)mouseState.y * cameraRotateSpeed, -cameraRotateLimit / 2.f, cameraRotateLimit / 2.f));
    camera.setRotation(rotation);

    grounded = false;
}

void Player::setCameraAspectRatio(UINT width, UINT height)
{
    camera.setAspectRatio(width, height);
}

void Player::setBreakBlockFunction(std::function<void(Segment ray)> function)
{
    breakBlock = function;
}

void Player::setPlaceBlockFunction(std::function<void(Segment ray)> function)
{
    placeBlock = function;
}
