#include "Enemy.hpp"

using namespace DirectX;

void Enemy::initialise(ID3D11Device* device, ID3D11DeviceContext* immediateContext)
{
    mesh.loadFromFile("models/character.obj");
    mesh.loadTexture(device, L"textures/Ghost-albedo.png", L"textures/Ghost-normal.png");
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

void Enemy::draw(ID3D11DeviceContext* immediateContext, std::vector<ID3D11Buffer*>& constantBuffers, VertexConstantBuffer vertexConstantBufferValue)
{
    mesh.setShaders(immediateContext);
    
    vertexConstantBufferValue.worldViewProjection = mesh.getTransform() * vertexConstantBufferValue.worldViewProjection;
    immediateContext->UpdateSubresource(constantBuffers[0], 0, 0, &vertexConstantBufferValue, 0, 0);
    immediateContext->VSSetConstantBuffers(0, 1, &constantBuffers[0]);
    immediateContext->PSSetConstantBuffers(0, 1, &constantBuffers[1]);

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

void Enemy::setPosition(const XMVECTOR& position)
{
	Character::setPosition(position);
	mesh.setPosition(position);
}

void Enemy::update(float deltaTime)
{
	Character::update(deltaTime);

	XMVECTOR positionOffset = XMVectorSet(0.f, velocity, 0.f, 0.f);
	move(positionOffset * deltaTime);

	jump();

	grounded = false;
}

void Enemy::moveTowards(XMVECTOR position, float deltaTime)
{
	XMVECTOR offset = position - this->position;
	offset = XMVectorSetY(offset, 0.f);
	offset = XMVector3Normalize(offset);
	move(offset * moveSpeed * deltaTime);

    mesh.setRotation(XMVectorSetY(mesh.getRotation(), 90.f - XMConvertToDegrees(atan2f(XMVectorGetZ(offset), XMVectorGetX(offset)))));
}
