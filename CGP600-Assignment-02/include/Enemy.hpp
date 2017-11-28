#pragma once

#include "Character.hpp"
#include "Mesh.hpp"
#include "ConstantBuffers.hpp"

class Enemy : public Character
{
    private:
        Mesh mesh;
    public:
        void initialise(ID3D11Device* device, ID3D11DeviceContext* immediateContext);
        void draw(ID3D11DeviceContext* immediateContext, ID3D11Buffer* constantBuffer0, ConstantBuffer0 constantBuffer0Value);
};
