#include "WorldManager.hpp"
#include "ConstantBuffers.hpp"
#include "Utility.hpp"
#include <random>
#include <WICTextureLoader.h>

int WorldManager::getBlockIndex(int x, int y, int z)
{
    return x + width * (y + height * z);
}

// Get the index of the first block intersecting a ray
int WorldManager::getBlockIndex(Segment ray)
{
    XMVECTOR playerPosition = player.getPosition();
    int playerX = (int)floor(XMVectorGetX(playerPosition));
    int playerY = (int)floor(XMVectorGetY(playerPosition));
    int playerZ = (int)floor(XMVectorGetZ(playerPosition));

    const int checkRange = 4;

    int currentBlock = -1;
    float currentTime = 1.f; // [0-1] Represents how far along the ray the collision occured

    // Only bother checking around the player in a small radius for performance reasons
    for (int x = Utility::max(playerX - checkRange, 0); x < Utility::min(playerX + checkRange, width); x++)
    {
        for (int y = Utility::max(playerY - checkRange, 0); y < Utility::min(playerY + checkRange, height); y++)
        {
            for (int z = Utility::max(playerZ - checkRange, 0); z < Utility::min(playerZ + checkRange, depth); z++)
            {
                // Make sure there's a block here
                std::unique_ptr<Block>& block = getBlock(x, y, z);
                if (!block) continue;

                // Re-use a single block object moved to the current position to save memory
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

void WorldManager::buildInstanceBuffer()
{
    // Hold up the render thread to prevent bad data being read
    std::lock_guard<std::mutex> guard(mutex);

    // Get rid of the old instance buffer
    if (instanceBuffer)
    {
        instanceBuffer->Release();
        instanceBuffer = nullptr;
    }

    instances.clear();

    // Loop through the world
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            for (int z = 0; z < depth; z++)
            {
                // If there's a block, make an instance
                if (blocks[getBlockIndex(x, y, z)])
                {
                    BlockInstance instance;
                    instance.position = XMFLOAT4((float)x, (float)y, (float)z, 0.f);
                    instance.textureId = getBlock(x, y, z)->textureId;
                    instances.push_back(instance);
                }
            }
        }
    }

    // Make the buffer
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

void WorldManager::handleCharacterCollision(Character& character)
{
    const int checkRange = 2;

    XMVECTOR characterPosition = character.getPosition();
    int characterX = (int)floor(XMVectorGetX(characterPosition));
    int characterY = (int)floor(XMVectorGetY(characterPosition));
    int characterZ = (int)floor(XMVectorGetZ(characterPosition));

    XMVECTOR hitDelta = XMVectorZero();

    for (int x = Utility::max(characterX - checkRange, 0); x < Utility::min(characterX + checkRange, width); x++)
    {
        for (int y = Utility::max(characterY - checkRange, 0); y < Utility::min(characterY + checkRange, height); y++)
        {
            for (int z = Utility::max(characterZ - checkRange, 0); z < Utility::min(characterZ + checkRange, depth); z++)
            {
                if (getBlock(x, y, z) == nullptr) continue;

                XMVECTOR blockPosition = XMVectorSet((float)x, (float)y, (float)z, 0.f);
                blockObject->setPosition(blockPosition);
                Hit hit = blockObject->testIntersection(character);
                if (hit.hit)
                {
                    hitDelta += hit.delta;

                    if (XMVectorGetY(hit.delta) != 0.f)
                    {
                        XMVECTOR difference = XMVectorAbs(character.getPosition() - blockPosition);
                        if (XMVectorGetX(difference) < XMVectorGetX(character.getSize()) - 0.01f && XMVectorGetZ(difference) < XMVectorGetZ(character.getSize()) - 0.01f)
                        {
                            if (XMVectorGetY(hit.delta) > 0.f)
                            {
                                character.setGrounded(true);
                            }
                            character.setVelocity(0.f);
                        }
                    }
                }
            }
        }
    }

    character.move(hitDelta);

    if (XMVectorGetY(character.getPosition()) < -10.f)
    {
        character.setPosition(XMVectorSet((float)width / 2.f, (float)height + 2.f, (float)depth / 2.f, 0.f));
        character.setVelocity(0.f);
    }
}

void WorldManager::generateBlock(int x, int y, int z)
{
    const float scaleFactor = 16.f;

    if (noiseGenerator.noise((float)x / scaleFactor, (float)y / scaleFactor, (float)z / scaleFactor) > 0.5f)
    {
        addBlock(x, y, z, { 0 });
    }
}

WorldManager::WorldManager() :
    blocks(width * height * depth),
    noiseGenerator(std::uniform_int_distribution<int>(0, 999999999)(std::random_device()))
{
    // Setup the directional light
    directionalLight.setDirection(DirectX::XMVector3Normalize(DirectX::XMVectorSet(-1.f, -1.f, 1.f, 0.f)));
    directionalLight.setColour(XMFLOAT4(0.8f, 0.8f, 0.8f, 1.f));
    directionalLight.setAmbientColour(XMFLOAT4(0.2f, 0.2f, 0.2f, 1.f));

    pointLight.setColour(XMFLOAT4(0.8f, 0.8f, 0.8f, 1.f));
    pointLight.setPosition(XMVectorZero());
    pointLight.setFalloff(30.f);
}

WorldManager::~WorldManager()
{
    for (ID3D11ShaderResourceView* texture : textures)
    {
        texture->Release();
    }

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
    player.setPosition(XMVectorSet((float)width / 2.f, (float)height + 2.f, (float)depth / 2.f, 1.f));

    blockObject = std::make_unique<BlockObject>(device, immediateContext);
    D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "INST_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "TEXID", 0, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };
    blockObject->getMesh()->loadShaders(L"shaders/blockShaders.hlsl", device, inputElementDescriptions, ARRAYSIZE(inputElementDescriptions));

    for (int i = 0; i < 10; i++)
    {
        enemies.push_back(std::move(std::make_unique<Enemy>()));
    }
    for (std::unique_ptr<Enemy>& enemy : enemies)
    {
        enemy->initialise(device, immediateContext);
        enemy->setPosition(XMVectorSet((float)width / 2.f, (float)height + 2.f, (float)depth / 2.f, 0.f));
    }

    ID3D11ShaderResourceView* texture = nullptr;

    CreateWICTextureFromFile(device, immediateContext, L"textures/dry-dirt2-albedo.png", NULL, &texture);
    textures.push_back(texture);
    CreateWICTextureFromFile(device, immediateContext, L"textures/dry-dirt2-normal.png", NULL, &texture);
    textures.push_back(texture);
    CreateWICTextureFromFile(device, immediateContext, L"textures/grass1-albedo.png", NULL, &texture);
    textures.push_back(texture);
    CreateWICTextureFromFile(device, immediateContext, L"textures/grass1-normal.png", NULL, &texture);
    textures.push_back(texture);

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            for (int z = 0; z < depth; z++)
            {
                generateBlock(x, y, z);
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

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            for (int z = 0; z < depth; z++)
            {
                if (getBlock(x, y, z) && ((y == height - 1) || !getBlock(x, y + 1, z)))
                {
                    getBlock(x, y, z)->textureId = 1;
                }
            }
        }
    }

    for (int block : toRemove)
    {
        removeBlock(block);
    }

    buildInstanceBuffer();
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

void WorldManager::renderFrame(ID3D11DeviceContext* immediateContext, std::vector<ID3D11Buffer*>& constantBuffers)
{
    std::lock_guard<std::mutex> guard(mutex);

    blockObject->getMesh()->setShaders(immediateContext);

    VertexConstantBuffer vertexConstantBufferValue = {
        player.getCamera()->getViewMatrix(),
        directionalLight.getAmbientColour()
    };
    PixelConstantBuffer pixelConstantBufferValue = {
        DirectX::XMVectorNegate(directionalLight.getDirection()),
        directionalLight.getColour(),
        pointLight.getPosition(),
        pointLight.getColour(),
        pointLight.getFalloff()
    };
    immediateContext->UpdateSubresource(constantBuffers[0], 0, 0, &vertexConstantBufferValue, 0, 0);
    immediateContext->VSSetConstantBuffers(0, 1, &constantBuffers[0]);
    immediateContext->UpdateSubresource(constantBuffers[1], 0, 0, &pixelConstantBufferValue, 0, 0);
    immediateContext->PSSetConstantBuffers(1, 1, &constantBuffers[1]);
    immediateContext->PSSetShaderResources(0, (UINT)textures.size(), textures.data());

    UINT strides[2] = {
        sizeof(Vertex),
        sizeof(BlockInstance)
    };
    UINT offsets[2] = { 0, 0 };

    UINT vertexCount;

    ID3D11Buffer* buffers[2] = {
        blockObject->getMesh()->getVertexBuffer(&vertexCount),
        instanceBuffer
    };

    immediateContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
    immediateContext->DrawInstanced(vertexCount, (UINT)instances.size(), 0, 0);

    for (std::unique_ptr<Enemy>& enemy : enemies)
    {
        enemy->draw(immediateContext, constantBuffers, vertexConstantBufferValue);
    }
}

void WorldManager::update(float deltaTime)
{
    player.update(deltaTime);

    handleCharacterCollision(player);

    for (std::unique_ptr<Enemy>& enemy : enemies)
    {
        enemy->update(deltaTime);
        enemy->moveTowards(player.getPosition(), deltaTime);

        handleCharacterCollision(*enemy);

        for (std::unique_ptr<Enemy>& otherEnemy : enemies)
        {
            Hit hit = enemy->testIntersection(*otherEnemy);
            if (hit.hit)
            {
                hit.delta = XMVectorSetY(hit.delta, 0.f);
                otherEnemy->move(hit.delta / 2.f);
                enemy->move(-hit.delta / 2.f);
            }
        }

        Hit hit = enemy->testIntersection(player);
        if (hit.hit)
        {
            hit.delta = XMVectorSetY(hit.delta, 0.f);
            player.move(hit.delta / 2.f);
            enemy->move(-hit.delta / 2.f);
            player.setVelocity(5.f);
        }
    }

    pointLight.setPosition(player.getPosition());
}

void WorldManager::setCameraAspectRatio(UINT width, UINT height)
{
    player.setCameraAspectRatio(width, height);
}
