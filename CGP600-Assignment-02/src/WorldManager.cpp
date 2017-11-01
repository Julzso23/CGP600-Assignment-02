#include "WorldManager.hpp"
#include "ConstantBuffers.hpp"
#include "PerlinNoise.hpp"

int WorldManager::getBlockIndex(int x, int y, int z)
{
    return x + width * (y + height * z);
}

void WorldManager::removeBlock(int index)
{
    blocks[index] = nullptr;
}

WorldManager::WorldManager() :
    width(32),
    height(32),
    depth(32),
    blocks(width * height * depth)
{
    light.setDirection(DirectX::XMVector3Normalize(DirectX::XMVectorSet(-1.f, -1.f, 1.f, 0.f)));
    light.setColour(DirectX::XMVectorSet(1.f, 1.f, 1.f, 0.f));
    light.setAmbientColour(DirectX::XMVectorSet(0.2f, 0.2f, 0.2f, 1.f));
}

void WorldManager::initialise(ID3D11Device* device, ID3D11DeviceContext* immediateContext)
{
    blockDetails[0] = std::move(std::make_unique<BlockDetails>(device, immediateContext, "Dirt", "dirt.bmp"));

    PerlinNoise noiseGenerator(1234);

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            for (int z = 0; z < depth; z++)
            {
                if (noiseGenerator.noise((float)x / (float)width, (float)y / (float)height, (float)z / (float)depth) > 0.4f)
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

void WorldManager::renderFrame(ID3D11DeviceContext* immediateContext, DirectX::XMMATRIX viewMatrix, ID3D11Buffer* constantBuffer0)
{
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            for (int z = 0; z < depth; z++)
            {
                Block* block = getBlock(x, y, z);

                if (!block) continue;

                Mesh* mesh = blockDetails[block->id]->getMesh();

                mesh->setPosition({ (float)x, (float)y, (float)z });

                ConstantBuffer0 constantBuffer0Value = {
                    mesh->getTransform() * viewMatrix,
                    DirectX::XMVectorNegate(light.getDirection()),
                    light.getColour(),
                    light.getAmbientColour()
                };
                immediateContext->UpdateSubresource(constantBuffer0, 0, 0, &constantBuffer0Value, 0, 0);
                immediateContext->VSSetConstantBuffers(0, 1, &constantBuffer0);

                UINT stride = sizeof(Vertex);
                UINT offset = 0;
                UINT vertexCount;
                ID3D11Buffer* vertexBuffer = mesh->getVertexBuffer(&vertexCount);
                ID3D11ShaderResourceView* texture = mesh->getTexture();
                immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
                immediateContext->PSSetShaderResources(0, 1, &texture);
                immediateContext->Draw(vertexCount, 0);
            }
        }
    }
}
