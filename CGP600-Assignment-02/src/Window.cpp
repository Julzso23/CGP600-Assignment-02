#include "Window.hpp"

HRESULT Window::initialiseWindow(HINSTANCE instance, int commandShow)
{
    return E_NOTIMPL;
}

bool Window::pollEvent(MSG* message)
{
    if (PeekMessage(message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(message);
        DispatchMessage(message);

        return false;
    }
    else
    {
        return true;
    }
}

Window::Window(HINSTANCE instance, int commandShow)
{
    initialiseWindow(instance, commandShow);
}
