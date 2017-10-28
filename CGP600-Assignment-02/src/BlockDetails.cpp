#include "BlockDetails.hpp"

BlockDetails::BlockDetails()
{
}

BlockDetails::BlockDetails(ID3D11Device* device, ID3D11DeviceContext* immediateContext, std::string name, std::string textureName) :
    name(name)
{
    mesh.loadFromFile("models/block.obj");
    textureName = "textures/" + textureName;
    mesh.loadTexture(device, std::wstring(textureName.begin(), textureName.end()).c_str());
    mesh.initialiseVertexBuffer(device, immediateContext);
}

std::string BlockDetails::getName()
{
    return name;
}

Mesh* BlockDetails::getMesh()
{
    return &mesh;
}
