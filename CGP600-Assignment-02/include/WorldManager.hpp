#pragma once

#include "Block.hpp"
#include "BlockDetails.hpp"
#include <memory>
#include <vector>
#include <map>

class WorldManager
{
    private:
        int width;
        int height;
        int depth;

        std::vector<std::unique_ptr<Block>> blocks;
        std::map<std::uint8_t, std::unique_ptr<BlockDetails>> blockDetails;
    public:
        WorldManager();
        void initialise(ID3D11Device* device, ID3D11DeviceContext* immediateContext);
        void addBlock(int x, int y, int z, Block value);
        void removeBlock(int x, int y, int z);
        Block* getBlock(int x, int y, int z);
        void renderFrame(ID3D11DeviceContext* immediateContext, DirectX::XMMATRIX viewMatrix, ID3D11Buffer* constantBuffer0);
};
