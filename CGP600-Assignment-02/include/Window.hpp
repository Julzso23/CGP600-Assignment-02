#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <chrono>
#include "Vertex.hpp"
#include "ConstantBuffers.hpp"
#include "Camera.hpp"
#include "WorldManager.hpp"

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
        ID3D11RenderTargetView* backBufferRTView = NULL;
        float* backgroundClearColour;
        ID3D11VertexShader* vertexShader;
        ID3D11PixelShader* pixelShader;
        ID3D11InputLayout* inputLayout;
        ID3D11Buffer* constantBuffer0;
        Camera camera;
        ID3D11DepthStencilView* zBuffer;
        ID3D11SamplerState* sampler0;
        std::chrono::high_resolution_clock::time_point lastTime;
        WorldManager worldManager;

        static LRESULT CALLBACK eventCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
        HRESULT initialiseD3D();
        HRESULT initialiseGraphics();
        void shutdownD3D();
        void setCursorClip(RECT windowRect, bool shouldClip);
    public:
        ~Window();
        bool pollMessage(MSG* message);
        HRESULT create(HINSTANCE instance, int commandShow, char* name);
        void update();
        void renderFrame();
        LRESULT eventCallbackInternal(UINT message, WPARAM wParam, LPARAM lParam);
};
