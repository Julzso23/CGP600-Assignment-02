#include "BlockObject.hpp"

BlockObject::BlockObject(ID3D11Device* device, ID3D11DeviceContext* immediateContext)
{
    mesh.loadFromFile("models/block.obj");
	mesh.initialiseVertexBuffer(device, immediateContext);

    setSize(DirectX::XMVectorSet(1.f, 1.f, 1.f, 0.f));
}

Mesh* BlockObject::getMesh()
{
    return &mesh;
}
