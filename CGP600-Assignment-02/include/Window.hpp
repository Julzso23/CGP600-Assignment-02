#pragma once

#include <Windows.h>

class Window
{
    private:
        HINSTANCE handleInstance = NULL;
        HWND window = NULL;
        static LRESULT CALLBACK eventCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    public:
        bool pollMessage(MSG* message);
        HRESULT create(HINSTANCE instance, int commandShow, char* name);
};
