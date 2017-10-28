#include "WorldManager.hpp"
#include "ConstantBuffers.hpp"

WorldManager::WorldManager() :
    width(32),
    height(16),
    depth(32),
    blocks(width * height * depth)
{
}

void WorldManager::initialise(ID3D11Device* device, ID3D11DeviceContext* immediateContext)
{
    blockDetails[0] = std::move(std::make_unique<BlockDetails>(device, immediateContext, "Dirt", "dirt.bmp"));

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            for (int z = 0; z < depth; z++)
            {
                addBlock(x, y, z, { 0 });
            }
        }
    }

    for (int x = 1; x < width - 1; x++)
    {
        for (int y = 1; y < height - 1; y++)
        {
            for (int z = 1; z < depth - 1; z++)
            {
                if (getBlock(x - 1, y, z) && getBlock(x + 1, y, z) &&
                    getBlock(x, y - 1, z), getBlock(x, y + 1, z) &&
                    getBlock(x, y, z - 1) && getBlock(x, y, z + 1))
                {
                    removeBlock(x, y, z);
                }
            }
        }
    }
}

void WorldManager::addBlock(int x, int y, int z, Block value)
{
    blocks[x + width * (y + height * z)] = std::make_unique<Block>(value);
}

void WorldManager::removeBlock(int x, int y, int z)
{
    blocks[x + width * (y + height * z)] = nullptr;
}

Block* WorldManager::getBlock(int x, int y, int z)
{
    return blocks[x  + width * (y + height * z)].get();
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

                ConstantBuffer0 constantBuffer0Value = { mesh->getTransform() * viewMatrix };
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
