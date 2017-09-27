#include "Window.hpp"
#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int commandShow)
{
    Window window;
    if (FAILED(window.create(instance, commandShow, "CGP600 Assignment 02\0")))
    {
        return 1;
    }

    bool running = true;
    MSG message = { 0 };
    while (running)
    {
        if (window.pollMessage(&message))
        {
            switch (message.message)
            {
                case WM_QUIT:
                {
                    running = false;
                    break;
                }
            }
        }
    }

    return message.wParam;
}
