#include "BlockDetails.hpp"

BlockDetails::BlockDetails()
{
}

BlockDetails::BlockDetails(ID3D11Device* device, ID3D11DeviceContext* immediateContext, std::string name, std::string textureName, std::string normalTextureName) :
    name(name)
{
    mesh.loadFromFile("models/block.obj");
    textureName = "textures/" + textureName;
    normalTextureName = "textures/" + normalTextureName;
    mesh.loadTexture(device, std::wstring(textureName.begin(), textureName.end()).c_str(), std::wstring(normalTextureName.begin(), normalTextureName.end()).c_str());

    setSize(DirectX::XMVectorSet(1.f, 1.f, 1.f, 0.f));
}

std::string BlockDetails::getName()
{
    return name;
}

Mesh* BlockDetails::getMesh()
{
    return &mesh;
}
