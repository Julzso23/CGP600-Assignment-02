#include "Player.hpp"

Player::Player()
{
    cameraOffset = XMVectorSet(0.f, 0.5f, 0.f, 0.f);

    camera.setFieldOfView(60.f);
    camera.setClippingPlanes(0.1f, 1000.f);
    camera.setPosition(position + cameraOffset);
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
}
