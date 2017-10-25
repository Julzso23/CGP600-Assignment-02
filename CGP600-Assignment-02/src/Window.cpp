#include "Window.hpp"
#include <stdio.h>
#include <d3dcompiler.h>

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

    D3D11_TEXTURE2D_DESC textureDescription;
    ZeroMemory(&textureDescription, sizeof(textureDescription));
    textureDescription.Width = width;
    textureDescription.Height = height;
    textureDescription.ArraySize = 1;
    textureDescription.MipLevels = 1;
    textureDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    textureDescription.SampleDesc.Count = swapChainDescription.SampleDesc.Count;
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

    return S_OK;
}

HRESULT Window::initialiseGraphics()
{
    HRESULT result;

    block.loadFromFile("models/monkey.obj");

    result = block.initialiseVertexBuffer(device, immediateContext);
    if (FAILED(result))
    {
        return result;
    }

    D3D11_BUFFER_DESC constantBuffer0Description;
    ZeroMemory(&constantBuffer0Description, sizeof(constantBuffer0Description));
    constantBuffer0Description.Usage = D3D11_USAGE_DEFAULT;
    constantBuffer0Description.ByteWidth = 64;
    constantBuffer0Description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    result = device->CreateBuffer(&constantBuffer0Description, NULL, &constantBuffer0);

    if (FAILED(result))
    {
        OutputDebugString("#### Failed to create constant buffer 0! ####\n");
        return result;
    }

    camera.setFieldOfView(60.f);
    camera.setClippingPlanes(0.1f, 100.f);
    camera.setPosition({ 0.f, 0.f, -2.f });

    ID3DBlob* vertShader = nullptr;
    ID3DBlob* pixShader = nullptr;
    ID3DBlob* error = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif

    // Vertex shader compile
    result = D3DCompileFromFile(L"shaders/shaders.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VShader", "vs_5_0", flags, NULL, &vertShader, &error);
    if (error != 0)
    {
        OutputDebugString((char*)error->GetBufferPointer());
        error->Release();
        if (FAILED(result))
        {
            OutputDebugString("#### Failed to compile vertex shader! ####\n");
            return result;
        }
    }

    // Pixel shader compile
    result = D3DCompileFromFile(L"shaders/shaders.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PShader", "ps_5_0", flags, NULL, &pixShader, &error);
    if (error != 0)
    {
        OutputDebugString((char*)error->GetBufferPointer());
        error->Release();
        if (FAILED(result))
        {
            OutputDebugString("#### Failed to compile pixel shader! ####\n");
            return result;
        }
    }

    // Vertex shader create
    result = device->CreateVertexShader(vertShader->GetBufferPointer(), vertShader->GetBufferSize(), NULL, &vertexShader);
    if (FAILED(result))
    {
        OutputDebugString("#### Failed to create vertex shader! ####\n");
        return result;
    }

    // Pixel shader create
    result = device->CreatePixelShader(pixShader->GetBufferPointer(), pixShader->GetBufferSize(), NULL, &pixelShader);
    if (FAILED(result))
    {
        OutputDebugString("#### Failed to create pixel shader! ####\n");
        return result;
    }

    immediateContext->VSSetShader(vertexShader, 0, 0);
    immediateContext->PSSetShader(pixelShader, 0, 0);

    D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    result = device->CreateInputLayout(inputElementDescriptions, 2, vertShader->GetBufferPointer(), vertShader->GetBufferSize(), &inputLayout);
    if (FAILED(result))
    {
        OutputDebugString("#### Failed to create input layout! ####\n");
        return result;
    }

    immediateContext->IASetInputLayout(inputLayout);

    return S_OK;
}

void Window::shutdownD3D()
{
    if (zBuffer) zBuffer->Release();
    if (constantBuffer0) constantBuffer0->Release();
    if (inputLayout) inputLayout->Release();
    if (vertexShader) vertexShader->Release();
    if (pixelShader) pixelShader->Release();
    if (backBufferRTView) backBufferRTView->Release();
    if (swapChain) swapChain->Release();
    if (immediateContext) immediateContext->Release();
    if (device) device->Release();
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

        return false;
    }
    else
    {
        return true;
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
    XMVECTOR rotation = camera.getRotation();
    rotation.vector4_f32[1] += 0.05f;
    camera.setRotation(rotation);

    XMVECTOR position;
    position.vector4_f32[0] = -2.f * (float)sin(XMConvertToRadians(rotation.vector4_f32[1]));
    position.vector4_f32[1] = 0.f;
    position.vector4_f32[2] = -2.f * (float)cos(XMConvertToRadians(rotation.vector4_f32[1]));
    camera.setPosition(position);
}

void Window::renderFrame()
{
    RECT rect;
    GetClientRect(window, &rect);
    UINT width = rect.right - rect.left;
    width = width != 0 ? width : 1;
    UINT height = rect.bottom - rect.top;
    height = height != 0 ? height : 1;

    camera.setAspectRatio(width, height);

    immediateContext->ClearRenderTargetView(backBufferRTView, backgroundClearColour);

    immediateContext->ClearDepthStencilView(zBuffer, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

    constantBuffer0Values.worldViewProjection = block.getTransform() * camera.getViewMatrix();
    immediateContext->UpdateSubresource(constantBuffer0, 0, 0, &constantBuffer0Values, 0, 0);
    immediateContext->VSSetConstantBuffers(0, 1, &constantBuffer0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    UINT vertexCount;
    ID3D11Buffer* vertexBuffer = block.getVertexBuffer(&vertexCount);
    immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    immediateContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    immediateContext->Draw(vertexCount, 0);

    swapChain->Present(1, 0);
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
                RECT rect;
                GetClientRect(window, &rect);
                UINT width = rect.right - rect.left;
                UINT height = rect.bottom - rect.top;

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
            break;
        }
        default:
        {
            return DefWindowProc(window, message, wParam, lParam);
        }
    }

    return 0;
}
