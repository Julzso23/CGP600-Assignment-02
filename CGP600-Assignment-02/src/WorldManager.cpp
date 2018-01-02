#include "WorldManager.hpp"
#include "ConstantBuffers.hpp"
#include "Utility.hpp"
#include <random>
#include <WICTextureLoader.h>

void WorldManager::blockRaytrace(Segment ray, int* blockIndexOut, Hit* hitOut)
{
    XMVECTOR cameraPosition = player.getCamera()->getPosition();

    // Get approximate block coordinate of the camera
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
                    // If this block is closer than the previous result
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

    // Make a reference to the instance array
    D3D11_SUBRESOURCE_DATA instanceData;
    ZeroMemory(&instanceData, sizeof(instanceData));
    instanceData.pSysMem = instances.data();

    // Create the buffer
    device->CreateBuffer(&bufferDescription, &instanceData, &instanceBuffer);
}

void WorldManager::handleCharacterCollision(Character& character)
{
    const int checkRange = 2;

    XMVECTOR characterPosition = character.getPosition();

    // Get approximate block coordinate of the character
    int characterX = (int)floor(XMVectorGetX(characterPosition));
    int characterY = (int)floor(XMVectorGetY(characterPosition));
    int characterZ = (int)floor(XMVectorGetZ(characterPosition));

    XMVECTOR hitDelta = XMVectorZero();

    // Only check in an area around the character for performance reasons
    for (int x = Utility::max(characterX - checkRange, 0); x < Utility::min(characterX + checkRange, width); x++)
    {
        for (int y = Utility::max(characterY - checkRange, 0); y < Utility::min(characterY + checkRange, height); y++)
        {
            for (int z = Utility::max(characterZ - checkRange, 0); z < Utility::min(characterZ + checkRange, depth); z++)
            {
                // Make sure the block exists
                if (getBlock(x, y, z) == nullptr) continue;

                XMVECTOR blockPosition = XMVectorSet((float)x, (float)y, (float)z, 1.f);

                // Re-use the block object for memory efficiency
                blockObject->setPosition(blockPosition);
                Hit hit = blockObject->testIntersection(character);
                if (hit.hit)
                {
                    hitDelta += hit.delta;

                    if (XMVectorGetY(hit.delta) != 0.f)
                    {
                        XMVECTOR difference = XMVectorAbs(character.getPosition() - blockPosition);
                        // Stop characters from jumping up sheer cliff faces
                        if (XMVectorGetX(difference) < XMVectorGetX(character.getSize()) - 0.001f && XMVectorGetZ(difference) < XMVectorGetZ(character.getSize()) - 0.001f)
                        {
                            // If the block is under the character, then it's grounded
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

    // Move the character out of any intersecting blocks
    character.move(hitDelta);

    // If the character fell out of the world
    if (XMVectorGetY(character.getPosition()) < -10.f)
    {
        // Reset their position and velocity
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

    // Setup the point light
    pointLight.setColour(XMFLOAT4(0.8f, 0.8f, 0.8f, 1.f));
    pointLight.setPosition(XMVectorZero());
    pointLight.setFalloff(30.f);
}

WorldManager::~WorldManager()
{
    // Cleanup the textures
    for (ID3D11ShaderResourceView* texture : textures)
    {
        texture->Release();
    }

    if (instanceBuffer) instanceBuffer->Release();
}

void WorldManager::initialise(HWND* windowHandle, ID3D11Device* device, ID3D11DeviceContext* immediateContext)
{
    // Keep references to the device and device context
    this->device = device;
    this->immediateContext = immediateContext;

    // Make a sprite batch and font
    spriteBatch = std::make_unique<SpriteBatch>(immediateContext);
    spriteFont = std::make_unique<SpriteFont>(device, L"fonts/comicsans.spritefont");

    // Setup the player
    player.initialise(windowHandle);
    player.setBreakBlockFunction([&](Segment ray)
    {
        int index;
        // Look for a block to break
        blockRaytrace(ray, &index, nullptr);
        // If there was a block in reach
        if (index != -1)
        {
            removeBlock(index);
            buildInstanceBuffer();
        }
    });
    player.setPlaceBlockFunction([&](Segment ray)
    {
        Hit hit;
        // Look for a block to build on
        blockRaytrace(ray, nullptr, &hit);
        // If there was a block in reach
        if (hit.hit)
        {
            // Check if there's space in the world
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

    // Set the point light as a child in the hierarchy
    player.addChild(&pointLight);
    pointLight.setLocalPosition(XMVectorSet(0.f, 1.f, 0.f, 1.f));

    // Initialise the block object
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

    // Initialise the skybox
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

    // Create 10 enemies
    for (int i = 0; i < 10; i++)
    {
        enemies.push_back(std::move(std::make_unique<Enemy>()));
    }
    for (std::unique_ptr<Enemy>& enemy : enemies)
    {
        enemy->initialise(device, immediateContext);
        enemy->setPosition(XMVectorSet((float)width / 2.f, (float)height + 2.f, (float)depth / 2.f, 1.f));
    }

    // Create the textures for the blocks
    ID3D11ShaderResourceView* texture = nullptr;

    CreateWICTextureFromFile(device, immediateContext, L"textures/dirt-albedo.png", NULL, &texture);
    textures.push_back(texture);
    CreateWICTextureFromFile(device, immediateContext, L"textures/dirt-normal.png", NULL, &texture);
    textures.push_back(texture);
    CreateWICTextureFromFile(device, immediateContext, L"textures/grass-albedo.png", NULL, &texture);
    textures.push_back(texture);
    CreateWICTextureFromFile(device, immediateContext, L"textures/grass-normal.png", NULL, &texture);
    textures.push_back(texture);

    // Generate block values using the compute shader
    perlinNoiseCompute.initialise(device, immediateContext, std::uniform_int_distribution<int>(0, 999999999)(std::random_device()));
    perlinNoiseCompute.run();

    std::vector<bool> blockValues = perlinNoiseCompute.getBlockValues();

    for (std::size_t i = 0; i < blocks.size(); i++)
    {
        // If there should be a block in this position
        if (blockValues[i])
        {
            blocks[i] = std::make_unique<Block>(Block{ 0 });
        }
    }

    // Hollow out the world for performance reasons
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

    // Use the block shaders
    blockObject->getMesh()->setShaders(immediateContext);

    // Set constant buffers
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

    // Draw the blocks
    immediateContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
    immediateContext->DrawInstanced(vertexCount, (UINT)instances.size(), 0, 0);

    // Draw the enemies
    for (std::unique_ptr<Enemy>& enemy : enemies)
    {
        enemy->draw(immediateContext, constantBuffers, vertexConstantBufferValue);
    }

    // Draw the skybox
    skybox.draw(immediateContext, constantBuffers, vertexConstantBufferValue);

    // Render UI
    spriteBatch->Begin();
    // Frame rate
    spriteFont->DrawString(spriteBatch.get(), (std::to_wstring((int)floor(1.f / deltaTime)) + L" fps").c_str(), XMFLOAT2(10.f, 10.f));
    // Controls
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

        // Enemy to enemy collision
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

        // Enemy to player collision
        Hit hit = enemy->testIntersection(player);
        if (hit.hit)
        {
            hit.delta = XMVectorSetY(hit.delta, 0.f);
            player.move(hit.delta / 2.f);
            enemy->move(-hit.delta / 2.f);
            // Throw the player in the air
            player.setVelocity(5.f);
        }
    }

    // Set the skybox to follow the player
    skybox.setPosition(player.getPosition());
}

void WorldManager::setCameraAspectRatio(UINT width, UINT height)
{
    player.setCameraAspectRatio(width, height);
}
