#pragma once

#include "Camera.hpp"
#include "Character.hpp"
#include <Keyboard.h>
#include <Mouse.h>
#include <functional>

class Player : public Character
{
    private:
        Camera camera;
        XMVECTOR cameraOffset;
        const float cameraRotateSpeed = 0.1f;
        const float cameraRotateLimit = 170.f; // Vertical limit of rotation in degrees
        const float reach = 3.f; // Max distance to interact with blocks
        const float fieldOfView = 60.f;
        const float sprintFieldOfView = 70.f;

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

        void setBreakBlockFunction(std::function<void(Segment ray)> function);
        void setPlaceBlockFunction(std::function<void(Segment ray)> function);
};
