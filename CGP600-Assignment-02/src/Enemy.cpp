#include "Enemy.hpp"
#include <random>

using namespace DirectX;

void Enemy::initialise(ID3D11Device* device, ID3D11DeviceContext* immediateContext)
{
    mesh.loadFromFile("models/character.obj");
    mesh.loadTexture(device, L"textures/ghost-albedo.png", L"textures/ghost-normal.png");
    mesh.initialiseVertexBuffer(device, immediateContext);

    D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    mesh.loadShaders(L"shaders/modelShaders.hlsl", device, inputElementDescriptions, ARRAYSIZE(inputElementDescriptions));

    std::random_device randomDevice;
    std::default_random_engine randomEngine(randomDevice());
    std::uniform_real_distribution<float> distribution(0.75f, 1.25f); // Random scale with range of 75% to 125%
    float scale = distribution(randomEngine);
    setSize(getSize() * scale);
    mesh.setScale(DirectX::XMVectorSet(scale, scale, scale, 0.f));
}

void Enemy::draw(ID3D11DeviceContext* immediateContext, std::vector<ID3D11Buffer*>& constantBuffers, VertexConstantBuffer vertexConstantBufferValue)
{
    mesh.draw(immediateContext, constantBuffers, vertexConstantBufferValue);
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
