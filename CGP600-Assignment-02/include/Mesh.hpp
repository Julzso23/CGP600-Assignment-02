#pragma once

#include "Vertex.hpp"
#include "ConstantBuffers.hpp"
#include <vector>
#include <d3d11.h>
#include <mutex>

class Mesh
{
    private:
        std::vector<Vertex> vertices;
        ID3D11Buffer* vertexBuffer = nullptr;
        std::vector<int> indices;
        ID3D11Buffer* indexBuffer = nullptr;

        ID3D11VertexShader* vertexShader = nullptr;
        ID3D11PixelShader* pixelShader = nullptr;
        ID3D11InputLayout* inputLayout = nullptr;
        ID3D11SamplerState* sampler0 = nullptr;

        ID3D11ShaderResourceView* texture = nullptr;
        ID3D11ShaderResourceView* normalMap = nullptr;

        DirectX::XMVECTOR position;
        DirectX::XMVECTOR rotation;
        DirectX::XMVECTOR scale;

		mutable std::mutex mutex;
    public:
        Mesh();
        ~Mesh();

        void loadFromFile(const char* fileName);
        std::vector<Vertex>* getVertices();

        HRESULT initialiseVertexBuffer(ID3D11Device* device, ID3D11DeviceContext* immediateContext);

        ID3D11Buffer* getVertexBuffer(UINT* vertexCount) const;

        HRESULT loadTexture(ID3D11Device* device, const wchar_t* fileName, const wchar_t* normalMapFileName);
        ID3D11ShaderResourceView* getTexture() const;
        ID3D11ShaderResourceView* getNormalMap() const;

        void loadShaders(LPWSTR path, ID3D11Device* device, D3D11_INPUT_ELEMENT_DESC* inputElementDescriptions, UINT descriptionCount);
        void setShaders(ID3D11DeviceContext* immediateContext);

        void setPosition(DirectX::XMVECTOR position);
        DirectX::XMVECTOR getPosition() const;

        void setRotation(DirectX::XMVECTOR rotation);
        DirectX::XMVECTOR getRotation() const;

        void setScale(DirectX::XMVECTOR scale);
        DirectX::XMVECTOR getScale() const;

        DirectX::XMMATRIX getTransform() const;

        void draw(ID3D11DeviceContext* immediateContext, std::vector<ID3D11Buffer*>& constantBuffers, VertexConstantBuffer vertexConstantBufferValue);
};
