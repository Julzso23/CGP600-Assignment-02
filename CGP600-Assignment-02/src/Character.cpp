#include "Character.hpp"

Character::Character()
{
    setSize(DirectX::XMVectorSet(0.6f, 1.8f, 0.6f, 0.f)); // Collider size
}

void Character::update(float deltaTime)
{
    // Add gravity and clamp to terminal velocity
    velocity += gravity * deltaTime;
    if (velocity < terminalVelocity)
    {
        velocity = terminalVelocity;
    }
}

void Character::setGrounded(bool value)
{
    grounded = value;
}

void Character::setVelocity(float value)
{
    velocity = value;
}

void Character::jump()
{
    // Make sure the player is on the ground before jumping
    if (grounded)
    {
        velocity = jumpForce;
    }
}

void Character::move(const DirectX::XMVECTOR& offset)
{
    setPosition(DirectX::XMVectorSetW(position + offset, 1.f));
}
