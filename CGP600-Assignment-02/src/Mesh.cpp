#include "Mesh.hpp"
#include "StringHelpers.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <Windows.h>
#include <WICTextureLoader.h>

using namespace DirectX;

HRESULT Mesh::initialiseVertexBuffer(ID3D11Device* device, ID3D11DeviceContext* immediateContext)
{
    D3D11_BUFFER_DESC vertexBufferDescription;
    ZeroMemory(&vertexBufferDescription, sizeof(vertexBufferDescription));
    vertexBufferDescription.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDescription.ByteWidth = vertices.size() * sizeof(Vertex);
    vertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT result = device->CreateBuffer(&vertexBufferDescription, NULL, &vertexBuffer);

    if (FAILED(result))
    {
        OutputDebugString("#### Failed to create vertex buffer! ####\n");
        return result;
    }

    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    immediateContext->Map(vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubresource);
    memcpy(mappedSubresource.pData, vertices.data(), vertices.size() * sizeof(Vertex));
    immediateContext->Unmap(vertexBuffer, NULL);

    return S_OK;
}

ID3D11Buffer* Mesh::getVertexBuffer(UINT* vertexCount) const
{
    *vertexCount = (UINT)vertices.size();
    return vertexBuffer;
}

HRESULT Mesh::loadTexture(ID3D11Device* device, ID3D11DeviceContext* immediateContext, const wchar_t* fileName)
{
    HRESULT result = CreateWICTextureFromFile(device, fileName, NULL, &texture);

    if (FAILED(result))
    {
        OutputDebugString("#### Failed to load texture! ####\n");
        return result;
    }

    return S_OK;
}

ID3D11ShaderResourceView * Mesh::getTexture()
{
    return texture;
}

Mesh::Mesh()
{
    position = DirectX::XMVectorZero();
    rotation = DirectX::XMVectorZero();
    scale = { 1.f, 1.f, 1.f };
}

Mesh::~Mesh()
{
    if (vertexBuffer) vertexBuffer->Release();
    if (texture) texture->Release();
}

void Mesh::loadFromFile(const char* fileName)
{
    std::ifstream file(fileName);

    if (!file)
    {
        OutputDebugString((std::string("#### Failed to load file: ") + std::string(fileName) + " ####\n").c_str());
    }

    std::vector<Vertex> newVertices;
    std::vector<XMFLOAT2> textureCoords;

    std::string line;
    while (std::getline(file, line))
    {
        std::vector<std::string> data = StringHelpers::split(line, ' ');

        if (data[0] == "v")
        {
            newVertices.push_back({
                XMFLOAT3(std::atof(data[1].c_str()), std::atof(data[2].c_str()), std::atof(data[3].c_str())),
                XMFLOAT4(1.f, 1.f, 1.f, 1.f)
            });

            continue;
        }

        if (data[0] == "vt")
        {
            textureCoords.push_back(XMFLOAT2(std::atof(data[1].c_str()), std::atof(data[2].c_str())));

            continue;
        }

        if (data[0] == "f")
        {
            for (int i = 1; i <= 3; i++)
            {
                std::vector<std::string> face = StringHelpers::split(data[i], '/');

                int vertexIndex = std::atoi(face[0].c_str());
                Vertex vertex = newVertices[vertexIndex - 1];

                int textureCoordsIndex = std::atoi(face[1].c_str());
                vertex.textureCoord = textureCoords[textureCoordsIndex - 1];

                vertices.push_back(vertex);
            }

            continue;
        }
    }

    file.close();
}

void Mesh::setPosition(XMVECTOR position)
{
    this->position = position;
}

XMVECTOR Mesh::getPosition() const
{
    return position;
}

void Mesh::setRotation(XMVECTOR rotation)
{
    this->rotation = rotation;
}

XMVECTOR Mesh::getRotation() const
{
    return rotation;
}

void Mesh::setScale(XMVECTOR scale)
{
    this->scale = scale;
}

XMVECTOR Mesh::getScale() const
{
    return scale;
}

XMMATRIX Mesh::getTransform() const
{
    XMMATRIX transform = XMMatrixIdentity();
    transform *= XMMatrixScalingFromVector(scale);
    transform *= XMMatrixRotationZ(XMConvertToRadians(rotation.vector4_f32[2]));
    transform *= XMMatrixRotationX(XMConvertToRadians(rotation.vector4_f32[0]));
    transform *= XMMatrixRotationY(XMConvertToRadians(rotation.vector4_f32[1]));
    transform *= XMMatrixTranslationFromVector(position);
    return transform;
}