#pragma once

#include "Block.hpp"
#include "BlockObject.hpp"
#include "Light.hpp"
#include "Player.hpp"
#include "BlockInstance.hpp"
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

        ID3D11Buffer* instanceBuffer = nullptr;
        std::vector<BlockInstance> instances;
        ID3D11Device* device;
        ID3D11DeviceContext* immediateContext;
		std::unique_ptr<BlockObject> blockObject;
		std::vector<ID3D11ShaderResourceView*> textures;
        mutable std::mutex mutex;

        Light light;

		Player player;

        int getBlockIndex(int x, int y, int z);
        int getBlockIndex(Segment ray);
        void removeBlock(int index);
        void buildInstanceBuffer();
    public:
        WorldManager();
        ~WorldManager();
        void initialise(HWND* windowHandle, ID3D11Device* device, ID3D11DeviceContext* immediateContext);
        void addBlock(int x, int y, int z, Block value);
        void removeBlock(int x, int y, int z);
		std::unique_ptr<Block>& getBlock(int x, int y, int z);
        void renderFrame(ID3D11DeviceContext* immediateContext, ID3D11Buffer* constantBuffer0);
        void update(float deltaTime);
        void setCameraAspectRatio(UINT width, UINT height);
};
