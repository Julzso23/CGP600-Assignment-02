#pragma once

#include "Character.hpp"
#include "Mesh.hpp"

class Enemy : public Character
{
    private:
        Mesh mesh;
    public:
        Mesh* getMesh();
        void draw(ID3D11DeviceContext* immediateContext);
};
