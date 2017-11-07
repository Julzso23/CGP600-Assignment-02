#pragma once

#include "Transformable.hpp"
#include "Camera.hpp"

class Player : public Transformable
{
    private:
        Camera camera;
        XMVECTOR cameraOffset;
    public:
        Player();

        Camera* getCamera();
        void setPosition(XMVECTOR position);
        void update(float deltaTime);
};
