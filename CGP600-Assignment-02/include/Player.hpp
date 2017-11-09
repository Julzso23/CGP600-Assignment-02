#pragma once

#include "Transformable.hpp"
#include "Camera.hpp"

class Player : public Transformable
{
    private:
        Camera camera;
        XMVECTOR cameraOffset;
		const float moveSpeed = 4.f;
		HWND* window;
    public:
        Player(HWND* windowHandle);

        Camera* getCamera();
        void setPosition(XMVECTOR position);
        void update(float deltaTime);
		void setCameraAspectRatio(UINT width, UINT height);
};
