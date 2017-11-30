#include "Enemy.hpp"

void Enemy::initialise(ID3D11Device* device, ID3D11DeviceContext* immediateContext)
{
    mesh.loadFromFile("models/character.obj");
    mesh.loadTexture(device, L"textures/dry-dirt2-albedo.png", L"textures/dry-dirt2-normal.png");
    mesh.initialiseVertexBuffer(device, immediateContext);

    D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    mesh.loadShaders(L"shaders/modelShaders.hlsl", device, inputElementDescriptions, ARRAYSIZE(inputElementDescriptions));
}

void Enemy::draw(ID3D11DeviceContext* immediateContext, ID3D11Buffer* constantBuffer0, ConstantBuffer0 constantBuffer0Value)
{
    mesh.setShaders(immediateContext);
    
    constantBuffer0Value.worldViewProjection = mesh.getTransform() * constantBuffer0Value.worldViewProjection;
    immediateContext->UpdateSubresource(constantBuffer0, 0, 0, &constantBuffer0Value, 0, 0);
    immediateContext->VSSetConstantBuffers(0, 1, &constantBuffer0);
    immediateContext->PSSetConstantBuffers(0, 1, &constantBuffer0);

    ID3D11ShaderResourceView* textures[] = {
        mesh.getTexture(),
        mesh.getNormalMap()
    };

    UINT vertexCount;
    ID3D11Buffer* vertexBuffer = mesh.getVertexBuffer(&vertexCount);

    UINT strides = sizeof(Vertex);
    UINT offsets = 0;

    immediateContext->PSSetShaderResources(0, 2, textures);
    immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offsets);
    immediateContext->Draw(vertexCount, 0);
}

void Enemy::setPosition(DirectX::XMVECTOR position)
{
	Character::setPosition(position);
	mesh.setPosition(position);
}
