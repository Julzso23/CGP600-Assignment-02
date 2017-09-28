#include "Window.hpp"
#include <stdio.h>

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int commandShow)
{
#if _DEBUG
    // Give us a console in debug mode
    AllocConsole(); // Create the console
    FILE* file;
    freopen_s(&file, "CONOUT$", "wb", stdout); // Pipe stdout into the console
#endif

    Window window;
    if (FAILED(window.create(instance, commandShow, "CGP600 Assignment 02\0")))
    {
        OutputDebugString("#### Failed to create window! ####\n");
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

        window.renderFrame();
    }

    return (int)message.wParam;
}
