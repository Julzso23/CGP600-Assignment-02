#include "WorldManager.hpp"
#include "ConstantBuffers.hpp"
#include "PerlinNoise.hpp"
#include "Utility.hpp"
#include <random>

int WorldManager::getBlockIndex(int x, int y, int z)
{
    return x + width * (y + height * z);
}

int WorldManager::getBlockIndex(Segment ray)
{
	XMVECTOR playerPosition = player.getPosition();
	int playerX = (int)floor(XMVectorGetX(playerPosition));
	int playerY = (int)floor(XMVectorGetY(playerPosition));
	int playerZ = (int)floor(XMVectorGetZ(playerPosition));

    const int checkRange = 4;

    int currentBlock = -1;
    float currentTime = 1.f;

    for (int x = Utility::max(playerX - checkRange, 0); x < Utility::min(playerX + checkRange, width); x++)
    {
        for (int y = Utility::max(playerY - checkRange, 0); y < Utility::min(playerY + checkRange, height); y++)
        {
            for (int z = Utility::max(playerZ - checkRange, 0); z < Utility::min(playerZ + checkRange, depth); z++)
            {
				std::unique_ptr<Block>& block = getBlock(x, y, z);
                if (!block) continue;

                std::shared_ptr<BlockDetails>& blockObject = block->details;
                blockObject->setPosition(XMVectorSet((float)x, (float)y, (float)z, 0.f));
                Hit hit = blockObject->testIntersection(ray);
                if (hit.hit)
                {
                    if (hit.time < currentTime)
                    {
                        currentTime = hit.time;
                        currentBlock = getBlockIndex(x, y, z);
                    }
                }
            }
        }
    }

    return currentBlock;
}

void WorldManager::removeBlock(int index)
{
    blocks[index] = nullptr;
}

void WorldManager::buildVertexBuffer()
{
    std::lock_guard<std::mutex> guard(mutex);

    if (vertexBuffer)
    {
        vertexBuffer->Release();
        vertexBuffer = nullptr;
    }

    vertices.clear();

    std::vector<Vertex>* blockVertices = blockDetails[0]->getMesh()->getVertices();
    vertices.insert(vertices.end(), blockVertices->begin(), blockVertices->end());

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

void WorldManager::buildInstanceBuffer()
{
    std::lock_guard<std::mutex> guard(mutex);

    if (instanceBuffer)
    {
        instanceBuffer->Release();
        instanceBuffer = nullptr;
    }

    instances.clear();

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            for (int z = 0; z < depth; z++)
            {
                if (blocks[getBlockIndex(x, y, z)])
                {
                    instances.push_back({ XMVectorSet((float)x, (float)y, (float)z, 0.f) });
                }
            }
        }
    }

    D3D11_BUFFER_DESC bufferDescription;
    ZeroMemory(&bufferDescription, sizeof(bufferDescription));
    bufferDescription.Usage = D3D11_USAGE_DEFAULT;
    bufferDescription.ByteWidth = sizeof(BlockInstance) * (UINT)instances.size();
    bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA instanceData;
    ZeroMemory(&instanceData, sizeof(instanceData));
    instanceData.pSysMem = instances.data();

    device->CreateBuffer(&bufferDescription, &instanceData, &instanceBuffer);
}

WorldManager::WorldManager() :
    width(64),
    height(64),
    depth(64),
    blocks(width * height * depth)
{
    light.setDirection(DirectX::XMVector3Normalize(DirectX::XMVectorSet(-1.f, -1.f, 1.f, 0.f)));
    light.setColour(DirectX::XMVectorSet(1.f, 1.f, 1.f, 1.f));
    light.setAmbientColour(DirectX::XMVectorSet(0.2f, 0.2f, 0.2f, 1.f));
}

WorldManager::~WorldManager()
{
    if (vertexBuffer) vertexBuffer->Release();
    if (instanceBuffer) instanceBuffer->Release();
}

void WorldManager::initialise(HWND* windowHandle, ID3D11Device* device, ID3D11DeviceContext* immediateContext)
{
    this->device = device;
    this->immediateContext = immediateContext;

    player.initialise(windowHandle);
    player.setBreakBlockFunction([&](Segment ray)
    {
        int index = getBlockIndex(ray);
        if (index != -1)
        {
            removeBlock(index);
            buildInstanceBuffer();
        }
    });
    player.setPlaceBlockFunction([&](Segment ray)
    {
    });
    player.setPosition(XMVectorSet((float)width / 2.f, (float)height + 2.f, (float)depth / 2.f, 0.f));

    blockDetails.push_back(std::move(std::make_shared<BlockDetails>(device, immediateContext, "Dirt", "harshbricks-albedo.png", "harshbricks-normal.png")));
    blockDetails[0]->getMesh()->initialiseVertexBuffer(device, immediateContext);

    PerlinNoise noiseGenerator(std::uniform_int_distribution<int>(0, 999999999)(std::random_device()));

    const float scaleFactor = 16.f;

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            for (int z = 0; z < depth; z++)
            {
                if (noiseGenerator.noise((float)x / scaleFactor, (float)y / scaleFactor, (float)z / scaleFactor) > 0.5f)
                {
                    addBlock(x, y, z, { blockDetails[0] });
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

    buildInstanceBuffer();

    buildVertexBuffer();
}

void WorldManager::addBlock(int x, int y, int z, Block value)
{
    blocks[getBlockIndex(x, y, z)] = std::make_unique<Block>(value);
}

void WorldManager::removeBlock(int x, int y, int z)
{
    removeBlock(getBlockIndex(x, y, z));
}

std::unique_ptr<Block>& WorldManager::getBlock(int x, int y, int z)
{
    return blocks[getBlockIndex(x, y, z)];
}

void WorldManager::renderFrame(ID3D11DeviceContext* immediateContext, ID3D11Buffer* constantBuffer0)
{
    std::lock_guard<std::mutex> guard(mutex);

    ConstantBuffer0 constantBuffer0Value = {
        player.getCamera()->getViewMatrix(),
        DirectX::XMVectorNegate(light.getDirection()),
        light.getColour(),
        light.getAmbientColour()
    };
    immediateContext->UpdateSubresource(constantBuffer0, 0, 0, &constantBuffer0Value, 0, 0);
    immediateContext->VSSetConstantBuffers(0, 1, &constantBuffer0);
    immediateContext->PSSetConstantBuffers(0, 1, &constantBuffer0);

    ID3D11ShaderResourceView* shaderResources[] = {
        blockDetails[0]->getMesh()->getTexture(),
        blockDetails[0]->getMesh()->getNormalMap()
    };
    immediateContext->PSSetShaderResources(0, ARRAYSIZE(shaderResources), &shaderResources[0]);

    UINT strides[2] = {
        sizeof(Vertex),
        sizeof(BlockInstance)
    };
    UINT offsets[2] = { 0, 0 };

    ID3D11Buffer* buffers[2] = {
        vertexBuffer,
        instanceBuffer
    };

    immediateContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
    immediateContext->DrawInstanced((UINT)vertices.size(), (UINT)instances.size(), 0, 0);
}

void WorldManager::update(float deltaTime)
{
    player.update(deltaTime);

    int playerX = (int)floor(XMVectorGetX(player.getPosition()));
    int playerY = (int)floor(XMVectorGetY(player.getPosition()));
    int playerZ = (int)floor(XMVectorGetZ(player.getPosition()));

    const int checkRange = 2;

    for (int x = Utility::max(playerX - checkRange, 0); x < Utility::min(playerX + checkRange, width); x++)
    {
        for (int y = Utility::max(playerY - checkRange, 0); y < Utility::min(playerY + checkRange, height); y++)
        {
            for (int z = Utility::max(playerZ - checkRange, 0); z < Utility::min(playerZ + checkRange, depth); z++)
            {
				std::unique_ptr<Block>& block = getBlock(x, y, z);
                if (!block) continue;

                std::shared_ptr<BlockDetails>& blockObject = block->details;
                XMVECTOR blockPosition = XMVectorSet((float)x, (float)y, (float)z, 0.f);
                blockObject->setPosition(blockPosition);
                Hit hit = blockObject->testIntersection(player);
                if (hit.hit)
                {
                    player.setPosition(player.getPosition() + hit.delta);
                    if (XMVectorGetY(hit.delta) != 0.f)
                    {
                        XMVECTOR difference = XMVectorAbs(player.getPosition() - blockPosition);
                        if (XMVectorGetX(difference) < 0.99f && XMVectorGetZ(difference) < 0.99f)
                        {
							if (XMVectorGetY(hit.delta) > 0.f)
							{
								player.setGrounded(true);
							}
							player.setVelocity(0.f);
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
