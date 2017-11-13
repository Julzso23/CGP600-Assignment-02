#pragma once

#include <string>
#include "Mesh.hpp"
#include "collision\AABB.hpp"

class BlockDetails : public AABB
{
    private:
        std::string name;
        Mesh mesh;
    public:
        BlockDetails();
        BlockDetails(ID3D11Device* device, ID3D11DeviceContext* immediateContext, std::string name, std::string textureName, std::string normalTextureName);
        std::string getName();
        Mesh* getMesh();
};
