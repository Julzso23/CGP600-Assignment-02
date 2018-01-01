#include "WorldManager.hpp"
#include "ConstantBuffers.hpp"
#include "Utility.hpp"
#include <random>
#include <WICTextureLoader.h>

void WorldManager::blockRaytrace(Segment ray, int* blockIndexOut, Hit* hitOut)
{
    XMVECTOR cameraPosition = player.getCamera()->getPosition();
    int cameraX = (int)floor(XMVectorGetX(cameraPosition));
    int cameraY = (int)floor(XMVectorGetY(cameraPosition));
    int cameraZ = (int)floor(XMVectorGetZ(cameraPosition));

    const int checkRange = 4;

    int currentBlock = -1;
    Hit currentHit;
    currentHit.hit = false;
    currentHit.time = 1.f;

    // Only bother checking around the player in a small radius for performance reasons
    for (int x = Utility::max(cameraX - checkRange, 0); x <= Utility::min(cameraX + checkRange, width - 1); x++)
    {
        for (int y = Utility::max(cameraY - checkRange, 0); y <= Utility::min(cameraY + checkRange, height - 1); y++)
        {
            for (int z = Utility::max(cameraZ - checkRange, 0); z <= Utility::min(cameraZ + checkRange, depth - 1); z++)
            {
                // Make sure there's a block here
                if (getBlock(x, y, z) == nullptr) continue;

                // Re-use a single block object moved to the current position to save memory
                blockObject->setPosition(XMVectorSet((float)x, (float)y, (float)z, 1.f));
                Hit hit = blockObject->testIntersection(ray);
                if (hit.hit)
                {
                    if (hit.time < currentHit.time)
                    {
                        currentHit = hit;
                        currentBlock = getBlockIndex(x, y, z);
                        currentHit.position = XMVectorSet((float)x, (float)y, (float)z, 1.f);
                    }
                }
            }
        }
    }

    if (blockIndexOut)
    {
        *blockIndexOut = currentBlock;
    }
    if (hitOut)
    {
        *hitOut = currentHit;
    }
}

int WorldManager::getBlockIndex(int x, int y, int z)
{
    return x + width * (y + height * z);
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

                XMVECTOR blockPosition = XMVectorSet((float)x, (float)y, (float)z, 1.f);
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
        character.setPosition(XMVectorSet((float)width / 2.f, (float)height + 2.f, (float)depth / 2.f, 1.f));
        character.setVelocity(0.f);
    }
}

WorldManager::WorldManager() :
    blocks(width * height * depth)
{
    // Setup the directional light
    directionalLight.setDirection(DirectX::XMVector3Normalize(DirectX::XMVectorSet(-1.f, -1.f, 1.f, 0.f)));
    directionalLight.setColour(XMFLOAT4(0.4f, 0.4f, 0.4f, 1.f));
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

    spriteBatch = std::make_unique<SpriteBatch>(immediateContext);
    spriteFont = std::make_unique<SpriteFont>(device, L"fonts/comicsans.spritefont");

    perlinNoiseCompute.initialise(device, immediateContext, std::uniform_int_distribution<int>(0, 999999999)(std::random_device()));
    perlinNoiseCompute.run();

    player.initialise(windowHandle);
    player.setBreakBlockFunction([&](Segment ray)
    {
        int index;
        blockRaytrace(ray, &index, nullptr);
        if (index != -1)
        {
            removeBlock(index);
            buildInstanceBuffer();
        }
    });
    player.setPlaceBlockFunction([&](Segment ray)
    {
        Hit hit;
        blockRaytrace(ray, nullptr, &hit);
        if (hit.hit)
        {
            XMVECTOR newPosition = hit.position + hit.normal;
            if (XMVectorGetX(newPosition) >= 0.f && XMVectorGetX(newPosition) < (float)width &&
                XMVectorGetY(newPosition) >= 0.f && XMVectorGetY(newPosition) < (float)height &&
                XMVectorGetZ(newPosition) >= 0.f && XMVectorGetZ(newPosition) < (float)depth)
            {
                addBlock((int)floor(XMVectorGetX(newPosition)), (int)floor(XMVectorGetY(newPosition)), (int)floor(XMVectorGetZ(newPosition)), { 0 });
                buildInstanceBuffer();
            }
        }
    });
    player.setPosition(XMVectorSet((float)width / 2.f, (float)height + 2.f, (float)depth / 2.f, 1.f));

    player.addChild(&pointLight);
    pointLight.setLocalPosition(XMVectorSet(0.f, 1.f, 0.f, 1.f));

    blockObject = std::make_unique<BlockObject>(device, immediateContext);
    D3D11_INPUT_ELEMENT_DESC blockInputElementDescriptions[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "INST_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "TEXID", 0, DXGI_FORMAT_R32_UINT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };
    blockObject->getMesh()->loadShaders(L"shaders/blockShaders.hlsl", device, blockInputElementDescriptions, ARRAYSIZE(blockInputElementDescriptions));

    skybox.loadFromFile("models/skybox.obj");
    skybox.loadTexture(device, L"textures/clouds-albedo.png", L"textures/clouds-normal.png");
    D3D11_INPUT_ELEMENT_DESC skyboxInputElementDescriptions[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    skybox.loadShaders(L"shaders/skyboxShaders.hlsl", device, skyboxInputElementDescriptions, ARRAYSIZE(skyboxInputElementDescriptions));
    skybox.initialiseVertexBuffer(device, immediateContext);

    /*for (int i = 0; i < 10; i++)
    {
        enemies.push_back(std::move(std::make_unique<Enemy>()));
    }*/
    for (std::unique_ptr<Enemy>& enemy : enemies)
    {
        enemy->initialise(device, immediateContext);
        enemy->setPosition(XMVectorSet((float)width / 2.f, (float)height + 2.f, (float)depth / 2.f, 1.f));
    }

    ID3D11ShaderResourceView* texture = nullptr;

    CreateWICTextureFromFile(device, immediateContext, L"textures/dirt-albedo.png", NULL, &texture);
    textures.push_back(texture);
    CreateWICTextureFromFile(device, immediateContext, L"textures/dirt-normal.png", NULL, &texture);
    textures.push_back(texture);
    CreateWICTextureFromFile(device, immediateContext, L"textures/grass-albedo.png", NULL, &texture);
    textures.push_back(texture);
    CreateWICTextureFromFile(device, immediateContext, L"textures/grass-normal.png", NULL, &texture);
    textures.push_back(texture);

    std::vector<bool> blockValues = perlinNoiseCompute.getBlockValues();

    for (std::size_t i = 0; i < blocks.size(); i++)
    {
        if (blockValues[i])
        {
            blocks[i] = std::make_unique<Block>(Block{ 0 });
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

void WorldManager::renderFrame(float deltaTime, std::vector<ID3D11Buffer*>& constantBuffers, ID3D11BlendState* blendState)
{
    std::lock_guard<std::mutex> guard(mutex);

    blockObject->getMesh()->setShaders(immediateContext);

    VertexConstantBuffer vertexConstantBufferValue = {
        player.getCamera()->getViewMatrix(),
        directionalLight.getAmbientColour(),
        DirectX::XMVectorNegate(directionalLight.getDirection()),
        directionalLight.getColour()
    };
    PixelConstantBuffer pixelConstantBufferValue = {
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

    skybox.draw(immediateContext, constantBuffers, vertexConstantBufferValue);

    // Render frame rate
    spriteBatch->Begin();
    spriteFont->DrawString(spriteBatch.get(), (std::to_wstring((int)floor(1.f / deltaTime)) + L" fps").c_str(), XMFLOAT2(10.f, 10.f));
    spriteFont->DrawString(spriteBatch.get(), L"W A S D to move", XMFLOAT2(10.f, 30.f));
    spriteFont->DrawString(spriteBatch.get(), L"Hold shift while moving to sprint", XMFLOAT2(10.f, 50.f));
    spriteFont->DrawString(spriteBatch.get(), L"Space to jump", XMFLOAT2(10.f, 70.f));
    spriteFont->DrawString(spriteBatch.get(), L"Move the mouse to rotate the camera", XMFLOAT2(10.f, 90.f));
    spriteFont->DrawString(spriteBatch.get(), L"Left mouse button to break a block", XMFLOAT2(10.f, 110.f));
    spriteFont->DrawString(spriteBatch.get(), L"Right mouse button to place a block", XMFLOAT2(10.f, 130.f));
    spriteBatch->End();
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

    skybox.setPosition(player.getPosition());
}

void WorldManager::setCameraAspectRatio(UINT width, UINT height)
{
    player.setCameraAspectRatio(width, height);
}
