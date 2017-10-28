#pragma once

#include <string>
#include "Mesh.hpp"

class BlockDetails
{
    private:
        std::string name;
        Mesh mesh;
    public:
        BlockDetails();
        BlockDetails(ID3D11Device* device, ID3D11DeviceContext* immediateContext, std::string name, std::string textureName);
        std::string getName();
        Mesh* getMesh();
};
