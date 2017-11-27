#pragma once

#include "Transformable.hpp"
#include "collision\AABB.hpp"

class Character : public AABB
{
    protected:
        const float moveSpeed = 4.f;
        const float gravity = -9.8f;
        const float jumpForce = 5.f;
        const float terminalVelocity = -54.f; // Maximum vertical velocity
        bool grounded = false; // Is the player touching the ground
        float velocity = 0.f; // Vertical velocity
    public:
        Character();
        virtual void update(float deltaTime);
        void setGrounded(bool value);
        void setVelocity(float value);
        void jump();
        void move(DirectX::XMVECTOR offset);
};
