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
        void draw(ID3D11DeviceContext* immediateContext, std::vector<ID3D11Buffer*>& constantBuffers, VertexConstantBuffer vertexConstantBufferValue);
		void setPosition(const DirectX::XMVECTOR& position);
		void update(float deltaTime);
		void moveTowards(DirectX::XMVECTOR position, float deltaTime);
};
