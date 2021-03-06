#include "Window.hpp"
#include <stdio.h>
#include <d3dcompiler.h>
#include <windowsx.h>

using namespace DirectX;

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
        OutputDebugString("#### Failed to initialise device and swap chain! ####\n");
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

    D3D11_TEXTURE2D_DESC textureDescription;
    ZeroMemory(&textureDescription, sizeof(textureDescription));
    textureDescription.Width = width;
    textureDescription.Height = height;
    textureDescription.ArraySize = 1;
    textureDescription.MipLevels = 1;
    textureDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    textureDescription.SampleDesc.Count = swapChainDescription.SampleDesc.Count;
    textureDescription.SampleDesc.Quality = swapChainDescription.SampleDesc.Quality;
    textureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    textureDescription.Usage = D3D11_USAGE_DEFAULT;

    ID3D11Texture2D* zBufferTexture;
    result = device->CreateTexture2D(&textureDescription, NULL, &zBufferTexture);

    if (FAILED(result))
    {
        OutputDebugString("#### Failed to create Z buffer texture! ####\n");
        return result;
    }
    
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescription;
    ZeroMemory(&depthStencilViewDescription, sizeof(depthStencilViewDescription));
    depthStencilViewDescription.Format = textureDescription.Format;
    depthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;

    device->CreateDepthStencilView(zBufferTexture, &depthStencilViewDescription, &zBuffer);
    zBufferTexture->Release();

    D3D11_DEPTH_STENCIL_DESC depthStencilStateDescription;
    ZeroMemory(&depthStencilStateDescription, sizeof(depthStencilStateDescription));
    // Depth test parameters
    depthStencilStateDescription.DepthEnable = true;
    depthStencilStateDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilStateDescription.DepthFunc = D3D11_COMPARISON_LESS;
    // Stencil test parameters
    depthStencilStateDescription.StencilEnable = true;
    depthStencilStateDescription.StencilReadMask = 0xFF;
    depthStencilStateDescription.StencilWriteMask = 0xFF;
    // Stencil operations if pixel is front-facing
    depthStencilStateDescription.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilStateDescription.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilStateDescription.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilStateDescription.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    // Stencil operations if pixel is back-facing
    depthStencilStateDescription.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilStateDescription.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilStateDescription.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilStateDescription.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    device->CreateDepthStencilState(&depthStencilStateDescription, &depthStencilState);

    immediateContext->OMSetDepthStencilState(depthStencilState, 1);
    immediateContext->OMSetRenderTargets(1, &backBufferRTView, zBuffer);

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

HRESULT Window::initialiseGraphics()
{
    HRESULT result;

    worldManager.initialise(&window, device, immediateContext);

    D3D11_BUFFER_DESC constantBufferDescription;
    ZeroMemory(&constantBufferDescription, sizeof(constantBufferDescription));
    constantBufferDescription.Usage = D3D11_USAGE_DEFAULT;
    constantBufferDescription.ByteWidth = sizeof(VertexConstantBuffer);
    constantBufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    constantBuffers.resize(2);
    result = device->CreateBuffer(&constantBufferDescription, NULL, &constantBuffers[0]);
    constantBufferDescription.ByteWidth = sizeof(PixelConstantBuffer);
    result = device->CreateBuffer(&constantBufferDescription, NULL, &constantBuffers[1]);

    if (FAILED(result))
    {
        OutputDebugString("#### Failed to create constant buffers! ####\n");
        return result;
    }

    D3D11_BLEND_DESC blendStateDescription;
    ZeroMemory(&blendStateDescription, sizeof(blendStateDescription));
    blendStateDescription.RenderTarget[0].BlendEnable = true;
    blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&blendStateDescription, &blendState);

    immediateContext->OMSetBlendState(blendState, NULL, 0xffffffff);

    return S_OK;
}

void Window::shutdownD3D()
{
    if (blendState) blendState->Release();
    if (zBuffer) zBuffer->Release();

    for (ID3D11Buffer* buffer : constantBuffers)
    {
        if (buffer)
        {
            buffer->Release();
        }
    }

    if (backBufferRTView) backBufferRTView->Release();
    if (swapChain) swapChain->Release();
    if (immediateContext) immediateContext->Release();
    if (device) device->Release();
}

Window::Window()
{
}

Window::~Window()
{
    shutdownD3D();
    delete[] backgroundClearColour;
}

bool Window::pollMessage(MSG* message)
{
    if (PeekMessage(message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(message);
        DispatchMessage(message);

        return true;
    }
    else
    {
        return false;
    }
}

HRESULT Window::create(HINSTANCE instance, int commandShow, char* name)
{
    backgroundClearColour = new float[4] { 0.f, 0.f, 0.f, 1.f };

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

    HRESULT result = initialiseD3D();
    if (FAILED(result))
    {
        return result;
    }

    result = initialiseGraphics();
    if (FAILED(result))
    {
        return result;
    }

    return S_OK;
}

void Window::update()
{
    std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - updateLastTime).count();
    updateLastTime = currentTime;

    if (deltaTime > 0.1f)
    {
        deltaTime = 0.1f;
    }

    worldManager.update(deltaTime);

    if ((GetKeyState(VK_MENU) & 0x8000) && (GetKeyState(VK_F4) & 0x8000))
    {
        PostQuitMessage(0);
    }
}

void Window::renderFrame()
{
    std::lock_guard<std::mutex> lock(mutex);

    std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - renderLastTime).count();
    renderLastTime = currentTime;

    immediateContext->OMSetDepthStencilState(depthStencilState, 1);
    immediateContext->OMSetRenderTargets(1, &backBufferRTView, zBuffer);
    immediateContext->ClearRenderTargetView(backBufferRTView, backgroundClearColour);
    immediateContext->ClearDepthStencilView(zBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

    worldManager.renderFrame(deltaTime, constantBuffers, blendState);

    swapChain->Present(0, 0);
}

LRESULT Window::eventCallbackInternal(UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT paintStruct;
    HDC hdc;

    RECT rect;
    GetClientRect(window, &rect);
    UINT width = rect.right - rect.left;
    UINT height = rect.bottom - rect.top;

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
                std::lock_guard<std::mutex> lock(mutex);

                immediateContext->OMSetRenderTargets(0, 0, 0);
                backBufferRTView->Release();

                HRESULT result = swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

                if (FAILED(result))
                {
                    OutputDebugString("#### Failed to resize buffers! ####");
                    return 0;
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

                zBuffer->Release();

                D3D11_TEXTURE2D_DESC textureDescription;
                ZeroMemory(&textureDescription, sizeof(textureDescription));
                textureDescription.Width = width;
                textureDescription.Height = height;
                textureDescription.ArraySize = 1;
                textureDescription.MipLevels = 1;
                textureDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
                textureDescription.SampleDesc.Count = 1;
                textureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
                textureDescription.Usage = D3D11_USAGE_DEFAULT;

                ID3D11Texture2D* zBufferTexture;
                result = device->CreateTexture2D(&textureDescription, NULL, &zBufferTexture);

                if (FAILED(result))
                {
                    OutputDebugString("#### Failed to create Z buffer texture! ####\n");
                    return result;
                }

                D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDescription;
                ZeroMemory(&depthStencilDescription, sizeof(depthStencilDescription));
                depthStencilDescription.Format = textureDescription.Format;
                depthStencilDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

                device->CreateDepthStencilView(zBufferTexture, &depthStencilDescription, &zBuffer);
                zBufferTexture->Release();

                immediateContext->OMSetRenderTargets(1, &backBufferRTView, zBuffer);

                D3D11_VIEWPORT viewport;
                viewport.TopLeftX = 0;
                viewport.TopLeftY = 0;
                viewport.Width = (float)width;
                viewport.Height = (float)height;
                viewport.MinDepth = 0.f;
                viewport.MaxDepth = 1.f;

                immediateContext->RSSetViewports(1, &viewport);
            }

            worldManager.setCameraAspectRatio(width, height);

            break;
        }
        case WM_ACTIVATEAPP:
        {
            Keyboard::ProcessMessage(message, wParam, lParam);
            Mouse::ProcessMessage(message, wParam, lParam);
            break;
        }
        case WM_INPUT:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEWHEEL:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_MOUSEHOVER:
        {
            Mouse::ProcessMessage(message, wParam, lParam);
            break;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            Keyboard::ProcessMessage(message, wParam, lParam);
            break;
        }
        default:
        {
            return DefWindowProc(window, message, wParam, lParam);
        }
    }

    return 0;
}
