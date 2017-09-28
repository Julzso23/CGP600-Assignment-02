#pragma once

#include <Windows.h>
#include <d3d11.h>

class Window
{
    private:
        HINSTANCE handleInstance = NULL;
        HWND window = NULL;
        D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
        ID3D11Device* device = NULL;
        ID3D11DeviceContext* immediateContext = NULL;
        IDXGISwapChain* swapChain = NULL;

        static LRESULT CALLBACK eventCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
        HRESULT initialiseD3D();
        void shutdownD3D();
    public:
        ~Window();
        bool pollMessage(MSG* message);
        HRESULT create(HINSTANCE instance, int commandShow, char* name);
};
