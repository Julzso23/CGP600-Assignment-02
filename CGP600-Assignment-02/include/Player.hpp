#pragma once

#include "collision/AABB.hpp"
#include "Camera.hpp"
#include <Keyboard.h>
#include <Mouse.h>

class Player : public AABB
{
    private:
        Camera camera;
        XMVECTOR cameraOffset;
		const float moveSpeed = 4.f;
        const float cameraRotateSpeed = 0.1f;
        const XMVECTOR gravity = XMVectorSet(0.f, -9.8f, 0.f, 0.f);
		HWND* window;
        std::unique_ptr<DirectX::Keyboard> keyboard;
        std::unique_ptr<DirectX::Mouse> mouse;
    public:
        void initialise(HWND* windowHandle);

        Camera* getCamera();
        void setPosition(XMVECTOR position);
        void update(float deltaTime);
		void setCameraAspectRatio(UINT width, UINT height);
};
