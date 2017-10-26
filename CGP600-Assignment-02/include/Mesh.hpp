#pragma once

#include "Vertex.hpp"
#include <vector>
#include <d3d11.h>

class Mesh
{
    private:
        std::vector<Vertex> vertices;
        ID3D11Buffer* vertexBuffer;

        ID3D11ShaderResourceView* texture;

        DirectX::XMVECTOR position;
        DirectX::XMVECTOR rotation;
        DirectX::XMVECTOR scale;
    public:
        Mesh();
        ~Mesh();

        void loadFromFile(const char* fileName);

        HRESULT initialiseVertexBuffer(ID3D11Device* device, ID3D11DeviceContext* immediateContext);

        ID3D11Buffer* getVertexBuffer(UINT* vertexCount) const;

        HRESULT loadTexture(ID3D11Device* device, ID3D11DeviceContext* immediateContext, const wchar_t* fileName);
        ID3D11ShaderResourceView* getTexture();

        void setPosition(DirectX::XMVECTOR position);
        DirectX::XMVECTOR getPosition() const;

        void setRotation(DirectX::XMVECTOR rotation);
        DirectX::XMVECTOR getRotation() const;

        void setScale(DirectX::XMVECTOR scale);
        DirectX::XMVECTOR getScale() const;

        DirectX::XMMATRIX getTransform() const;
};