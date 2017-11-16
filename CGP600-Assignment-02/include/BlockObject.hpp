#pragma once

#include <string>
#include "Mesh.hpp"
#include "collision\AABB.hpp"

class BlockObject : public AABB
{
    private:
        Mesh mesh;
    public:
		BlockObject(ID3D11Device* device, ID3D11DeviceContext* immediateContext);
        Mesh* getMesh();
};
