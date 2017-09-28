#include "Window.hpp"
#include <stdio.h>

LRESULT Window::eventCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    Window* windowPointer = (Window*)GetWindowLongPtr(window, GWLP_USERDATA);
    if (windowPointer)
    {
        return windowPointer->eventCallbackInternal(message, wParam, lParam);
    }
    else
    {
        return DefWindowProc(window, message, wParam, lParam);
    }
}

HRESULT Window::initialiseD3D()
{
    HRESULT result = S_OK;

    RECT rect;
    GetClientRect(window, &rect);
    UINT width = rect.right - rect.left;
    UINT height = rect.bottom - rect.top;

    UINT createDeviceFlags = 0;

#if _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC swapChainDescription;
    ZeroMemory(&swapChainDescription, sizeof(swapChainDescription));
    swapChainDescription.BufferCount = 1;
    swapChainDescription.BufferDesc.Width = width;
    swapChainDescription.BufferDesc.Height = height;
    swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDescription.OutputWindow = window;
    swapChainDescription.SampleDesc.Count = 1;
    swapChainDescription.SampleDesc.Quality = 0;
    swapChainDescription.Windowed = true;

    for (UINT i = 0; i < numDriverTypes; ++i)
    {
        driverType = driverTypes[i];
        result = D3D11CreateDeviceAndSwapChain(
            NULL,
            driverType,
            NULL,
            createDeviceFlags,
            featureLevels,
            numFeatureLevels,
            D3D11_SDK_VERSION,
            &swapChainDescription,
            &swapChain,
            &device,
            &featureLevel,
            &immediateContext
        );

        if (SUCCEEDED(result))
        {
            break;
        }
    }

    if (FAILED(result))
    {
        OutputDebugString("#### Failed to initialise Direct3D! ####\n");
        return result;
    }

    ID3D11Texture2D* backBufferTexture;
    result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);

    if (FAILED(result))
    {
        OutputDebugString("#### Failed to create back buffer texture! ####\n");
        return result;
    }

    result = device->CreateRenderTargetView(backBufferTexture, NULL, &backBufferRTView);

    backBufferTexture->Release();

    if (FAILED(result))
    {
        OutputDebugString("#### Failed to create render target view! ####\n");
        return result;
    }

    immediateContext->OMSetRenderTargets(1, &backBufferRTView, NULL);

    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)width;
    viewport.Height = (float)height;
    viewport.MinDepth = 0.f;
    viewport.MaxDepth = 1.f;

    immediateContext->RSSetViewports(1, &viewport);

    return S_OK;
}

void Window::shutdownD3D()
{
    if (backBufferRTView)
    {
        backBufferRTView->Release();
    }
    if (swapChain)
    {
        swapChain->Release();
    }
    if (immediateContext)
    {
        immediateContext->Release();
    }
    if (device)
    {
        device->Release();
    }
}

Window::~Window()
{
    shutdownD3D();
}

bool Window::pollMessage(MSG* message)
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

HRESULT Window::create(HINSTANCE instance, int commandShow, char* name)
{
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = eventCallback;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = name;

    if (!RegisterClassEx(&windowClass))
    {
        return E_FAIL;
    }

    handleInstance = instance;
    RECT rect = { 0, 0, 1280, 720 };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    window = CreateWindow(
        name,
        name,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        instance,
        NULL
    );

    if (!window)
    {
        return E_FAIL;
    }

    SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)this);

    ShowWindow(window, commandShow);

    return initialiseD3D();
}

void Window::renderFrame()
{
    immediateContext->ClearRenderTargetView(backBufferRTView, backgroundClearColour);

    // Render here

    swapChain->Present(0, 0);
}

LRESULT Window::eventCallbackInternal(UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT paintStruct;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
        {
            hdc = BeginPaint(window, &paintStruct);
            EndPaint(window, &paintStruct);
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        case WM_SIZE:
        {
            if (swapChain)
            {
            }
            break;
        }
        default:
        {
            return DefWindowProc(window, message, wParam, lParam);
        }
    }

    return 0;
}
