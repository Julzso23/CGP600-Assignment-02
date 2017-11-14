#pragma once

#include "collision/AABB.hpp"
#include "Camera.hpp"
#include <Keyboard.h>
#include <Mouse.h>
#include <functional>

class Player : public AABB
{
    private:
        Camera camera;
        XMVECTOR cameraOffset;
		const float moveSpeed = 4.f;
        const float cameraRotateSpeed = 0.1f;
        const float gravity = -9.8f;
        const float jumpForce = 5.f;

        bool grounded = false;
        float velocity = 0.f;

        bool leftMouseButtonDown = false;
        std::function<void(Segment ray)> breakBlock;
        bool rightMouseButtonDown = false;
        std::function<void(Segment ray)> placeBlock;

		HWND* window;

        std::unique_ptr<DirectX::Keyboard> keyboard;
        std::unique_ptr<DirectX::Mouse> mouse;
    public:
        void initialise(HWND* windowHandle);

        Camera* getCamera();
        void setPosition(XMVECTOR position);
        void update(float deltaTime);
		void setCameraAspectRatio(UINT width, UINT height);
        void setGrounded(bool value);

        void setBreakBlockFunction(std::function<void(Segment ray)> function);
        void setPlaceBlockFunction(std::function<void(Segment ray)> function);
};
