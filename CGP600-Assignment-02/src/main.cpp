#include "Window.hpp"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int commandShow)
{
    Window window(instance, commandShow);

    bool running = true;
    MSG message = { 0 };
    while (running)
    {
        if (window.pollEvent(&message))
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

    return 0;
}
