#include "WorldManager.hpp"
#include "ConstantBuffers.hpp"
#include "PerlinNoise.hpp"
#include "Utility.hpp"
#include <random>

int WorldManager::getBlockIndex(int x, int y, int z)
{
    return x + width * (y + height * z);
}

void WorldManager::removeBlock(int index)
{
    blocks[index] = nullptr;
}

WorldManager::WorldManager() :
    width(64),
    height(32),
    depth(64),
    blocks(width * height * depth)
{
    light.setDirection(DirectX::XMVector3Normalize(DirectX::XMVectorSet(-1.f, -1.f, 1.f, 0.f)));
    light.setColour(DirectX::XMVectorSet(1.f, 1.f, 1.f, 1.f));
    light.setAmbientColour(DirectX::XMVectorSet(0.2f, 0.2f, 0.2f, 1.f));
}

void WorldManager::initialise(HWND* windowHandle, ID3D11Device* device, ID3D11DeviceContext* immediateContext)
{
    player.initialise(windowHandle);
    player.setPosition(XMVectorSet((float)width / 2.f, (float)height + 2.f, (float)depth / 2.f, 0.f));

    blockDetails[0] = std::move(std::make_unique<BlockDetails>(device, immediateContext, "Dirt", "harshbricks-albedo.png", "harshbricks-normal.png"));

    PerlinNoise noiseGenerator(std::uniform_int_distribution<int>(0, 999999999)(std::random_device()));

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            for (int z = 0; z < depth; z++)
            {
                if (noiseGenerator.noise((float)x / (float)width, (float)y / (float)height, (float)z / (float)depth) > 0.5f)
                {
                    addBlock(x, y, z, { 0 });
                }
            }
        }
    }

    std::vector<int> toRemove;

    for (int x = 1; x < width - 1; x++)
    {
        for (int y = 1; y < height - 1; y++)
        {
            for (int z = 1; z < depth - 1; z++)
            {
                if (getBlock(x - 1, y, z) && getBlock(x + 1, y, z) &&
                    getBlock(x, y - 1, z) && getBlock(x, y + 1, z) &&
                    getBlock(x, y, z - 1) && getBlock(x, y, z + 1))
                {
                    toRemove.push_back(getBlockIndex(x, y, z));
                }
            }
        }
    }

    for (int block : toRemove)
    {
        removeBlock(block);
    }

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            for (int z = 0; z < depth; z++)
            {
                Block* block = getBlock(x, y, z);
                if (!block) continue;

                Mesh* mesh = blockDetails[block->id]->getMesh();
                mesh->setPosition(DirectX::XMVectorSet((float)x, (float)y, (float)z, 0.f));

                std::vector<Vertex>* meshVertices = blockDetails[0]->getMesh()->getVertices();
                for (Vertex vertex : *meshVertices)
                {
                    Vertex newVertex = vertex;
                    newVertex.position = (DirectX::XMFLOAT4)DirectX::XMVector3Transform(DirectX::XMLoadFloat4(&vertex.position), mesh->getTransform()).vector4_f32;
                    vertices.push_back(newVertex);
                }
            }
        }
    }

    D3D11_BUFFER_DESC vertexBufferDescription;
    ZeroMemory(&vertexBufferDescription, sizeof(vertexBufferDescription));
    vertexBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDescription.ByteWidth = (UINT)(vertices.size() * sizeof(Vertex));
    vertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT result = device->CreateBuffer(&vertexBufferDescription, NULL, &vertexBuffer);

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    immediateContext->Map(vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
    memcpy(mappedSubresource.pData, vertices.data(), vertices.size() * sizeof(Vertex));
    immediateContext->Unmap(vertexBuffer, NULL);
}

void WorldManager::addBlock(int x, int y, int z, Block value)
{
    blocks[getBlockIndex(x, y, z)] = std::make_unique<Block>(value);
}

void WorldManager::removeBlock(int x, int y, int z)
{
    removeBlock(getBlockIndex(x, y, z));
}

Block* WorldManager::getBlock(int x, int y, int z)
{
    return blocks[getBlockIndex(x, y, z)].get();
}

void WorldManager::renderFrame(ID3D11DeviceContext* immediateContext, ID3D11Buffer* constantBuffer0)
{
    ConstantBuffer0 constantBuffer0Value = {
        player.getCamera()->getViewMatrix(),
        DirectX::XMVectorNegate(light.getDirection()),
        light.getColour(),
        light.getAmbientColour()
    };
    immediateContext->UpdateSubresource(constantBuffer0, 0, 0, &constantBuffer0Value, 0, 0);
    immediateContext->VSSetConstantBuffers(0, 1, &constantBuffer0);
    immediateContext->PSSetConstantBuffers(0, 1, &constantBuffer0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ID3D11ShaderResourceView* shaderResources[] = {
        blockDetails[0]->getMesh()->getTexture(),
        blockDetails[0]->getMesh()->getNormalMap()
    };
    immediateContext->PSSetShaderResources(0, ARRAYSIZE(shaderResources), &shaderResources[0]);
    immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    immediateContext->Draw((UINT)vertices.size(), 0);
}

void WorldManager::update(float deltaTime)
{
    player.update(deltaTime);

    int playerX = (int)floor(XMVectorGetX(player.getPosition()));
    int playerY = (int)floor(XMVectorGetY(player.getPosition()));
    int playerZ = (int)floor(XMVectorGetZ(player.getPosition()));

    for (int x = Utility::clamp(playerX - 2, 0, width - 1); x <= Utility::clamp(playerX + 2, 0, width - 1); x++)
    {
        for (int y = Utility::clamp(playerY - 2, 0, height - 1); y <= Utility::clamp(playerY + 2, 0, height - 1); y++)
        {
            for (int z = Utility::clamp(playerZ - 2, 0, depth - 1); z <= Utility::clamp(playerZ + 2, 0, depth - 1); z++)
            {
                Block* block = getBlock(x, y, z);
                if (!block) continue;

                std::unique_ptr<BlockDetails>& blockObject = blockDetails[block->id];
                XMVECTOR blockPosition = XMVectorSet((float)x, (float)y, (float)z, 0.f);
                blockObject->setPosition(blockPosition);
                Hit hit = blockObject->testIntersection(player);
                if (hit.hit)
                {
                    player.setPosition(player.getPosition() + hit.delta);
                    if (XMVectorGetY(hit.delta) > 0.f)
                    {
                        XMVECTOR difference = XMVectorAbs(player.getPosition() - blockPosition);
                        if (XMVectorGetX(difference) < 0.99f && XMVectorGetZ(difference) < 0.99f)
                        {
                            player.setGrounded(true);
                        }
                    }
                }
            }
        }
    }
}

void WorldManager::setCameraAspectRatio(UINT width, UINT height)
{
    player.setCameraAspectRatio(width, height);
}
