#pragma once

#include <Windows.h>

class Window
{
private:
    HRESULT initialiseWindow(HINSTANCE instance, int commandShow);
public:
    Window(HINSTANCE instance, int commandShow);
    bool pollEvent(MSG* message);
};
