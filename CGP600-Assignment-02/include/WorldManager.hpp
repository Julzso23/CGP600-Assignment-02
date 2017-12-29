#pragma once

#include "Block.hpp"
#include "BlockObject.hpp"
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "BlockInstance.hpp"
#include "PerlinNoise.hpp"
#include "PerlinNoiseCompute.hpp"
#include <memory>
#include <vector>
#include <map>
#include <SpriteFont.h>

class WorldManager
{
    private:
        const int width = 64;
        const int height = 64;
        const int depth = 64;

        //PerlinNoise noiseGenerator;

        std::vector<std::unique_ptr<Block>> blocks;

        ID3D11Buffer* instanceBuffer = nullptr;
        std::vector<BlockInstance> instances;
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* immediateContext = nullptr;
        std::unique_ptr<BlockObject> blockObject;
        Mesh skybox;
        std::vector<ID3D11ShaderResourceView*> textures;
        std::unique_ptr<SpriteBatch> spriteBatch;
        std::unique_ptr<SpriteFont> spriteFont;
        PerlinNoiseCompute perlinNoiseCompute;
        mutable std::mutex mutex;

        DirectionalLight directionalLight;
        PointLight pointLight;

        Player player;
        std::vector<std::unique_ptr<Enemy>> enemies;

        int getBlockIndex(int x, int y, int z);
        int getBlockIndex(Segment ray);
        void removeBlock(int index);
        void buildInstanceBuffer();
        void handleCharacterCollision(Character& character);
        //void generateBlock(int x, int y, int z);
    public:
        WorldManager();
        ~WorldManager();
        void initialise(HWND* windowHandle, ID3D11Device* device, ID3D11DeviceContext* immediateContext);
        void addBlock(int x, int y, int z, Block value);
        void removeBlock(int x, int y, int z);
        std::unique_ptr<Block>& getBlock(int x, int y, int z);
        void renderFrame(float deltaTime, std::vector<ID3D11Buffer*>& constantBuffers, ID3D11BlendState* blendState);
        void update(float deltaTime);
        void setCameraAspectRatio(UINT width, UINT height);
};
