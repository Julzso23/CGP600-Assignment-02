#include "Mesh.hpp"
#include "Utility.hpp"
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
    vertexBufferDescription.ByteWidth = (UINT)(vertices.size() * sizeof(Vertex));
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

HRESULT Mesh::loadTexture(ID3D11Device* device, const wchar_t* fileName, const wchar_t* normalMapFileName)
{
    HRESULT result = CreateWICTextureFromFile(device, fileName, NULL, &texture);

    if (FAILED(result))
    {
        OutputDebugString("#### Failed to load texture! ####\n");
        return result;
    }

    result = CreateWICTextureFromFile(device, normalMapFileName, NULL, &normalMap);

    if (FAILED(result))
    {
        OutputDebugString("#### Failed to load normal map! ####\n");
        return result;
    }

    return S_OK;
}

ID3D11ShaderResourceView* Mesh::getTexture() const
{
    return texture;
}

ID3D11ShaderResourceView* Mesh::getNormalMap() const
{
    return normalMap;
}

Mesh::Mesh()
{
    position = DirectX::XMVectorZero();
    rotation = DirectX::XMVectorZero();
    scale = XMVectorSet(1.f, 1.f, 1.f, 0.f);
}

Mesh::~Mesh()
{
    if (vertexBuffer) vertexBuffer->Release();
    if (texture) texture->Release();
    if (normalMap) normalMap->Release();
}

static void float3Normalize(XMFLOAT3* value)
{
    XMStoreFloat3(value, XMVector3Normalize(XMLoadFloat3(value)));
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
	std::vector<XMFLOAT3> normals;

    std::string line;
    while (std::getline(file, line))
    {
        std::vector<std::string> data = Utility::split(line, ' ');

        if (data[0] == "v")
        {
			Vertex vertex;
			ZeroMemory(&vertex, sizeof(Vertex));
			vertex.position = XMFLOAT4((float)std::atof(data[1].c_str()), (float)std::atof(data[2].c_str()), (float)std::atof(data[3].c_str()), 1.f);
			vertex.colour = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
            newVertices.push_back(vertex);

            continue;
        }

        if (data[0] == "vt")
        {
            textureCoords.push_back(XMFLOAT2((float)std::atof(data[1].c_str()), (float)std::atof(data[2].c_str())));

            continue;
        }

		if (data[0] == "vn")
		{
			normals.push_back(XMFLOAT3((float)std::atof(data[1].c_str()), (float)std::atof(data[2].c_str()), (float)std::atof(data[3].c_str())));

			continue;
		}

        if (data[0] == "f")
        {
            std::vector<Vertex> face(3);

            for (int i = 1; i <= 3; i++)
            {
                std::vector<std::string> vertexData = Utility::split(data[i], '/');

                int vertexIndex = std::atoi(vertexData[0].c_str());
                Vertex vertex = newVertices[vertexIndex - 1];

                int textureCoordsIndex = std::atoi(vertexData[1].c_str());
                vertex.textureCoord = textureCoords[textureCoordsIndex - 1];

				int normalIndex = std::atoi(vertexData[2].c_str());
				vertex.normal = normals[normalIndex - 1];

                face.push_back(vertex);
            }

            XMFLOAT3 vector0 = {
                face[1].position.x - face[0].position.x,
                face[1].position.y - face[0].position.y,
                face[1].position.z - face[0].position.z
            };
            XMFLOAT3 vector1 = {
                face[2].position.x - face[0].position.x,
                face[2].position.y - face[0].position.y,
                face[2].position.z - face[0].position.z
            };
            XMFLOAT2 uVector = {
                face[1].textureCoord.x - face[0].textureCoord.x,
                face[2].textureCoord.x - face[0].textureCoord.x
            };
            XMFLOAT2 vVector = {
                face[1].textureCoord.y - face[0].textureCoord.y,
                face[2].textureCoord.y - face[0].textureCoord.y
            };

            float denominator = 1.f / ((uVector.x * vVector.y) - (uVector.y * vVector.x));
            XMFLOAT3 tangent = {
                ((vVector.y * vector0.x) - (vVector.x * vector1.x)) * denominator,
                ((vVector.y * vector0.y) - (vVector.x * vector1.y)) * denominator,
                ((vVector.y * vector0.z) - (vVector.x * vector1.z)) * denominator
            };

            XMFLOAT3 binormal = {
                ((uVector.x * vector1.x) - (uVector.y * vector0.x)) * denominator,
                ((uVector.x * vector1.y) - (uVector.y * vector0.y)) * denominator,
                ((uVector.x * vector1.z) - (uVector.y * vector0.z)) * denominator
            };

            float3Normalize(&tangent);
            float3Normalize(&binormal);

            for (int i = 0; i < 3; i++)
            {
                face[i].tangent = tangent;
                face[i].binormal = binormal;
            }

            vertices.insert(vertices.end(), face.begin(), face.end());

            continue;
        }
    }

    file.close();
}

std::vector<Vertex>* Mesh::getVertices()
{
    return &vertices;
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
    transform *= XMMatrixRotationZ(XMConvertToRadians(XMVectorGetZ(rotation)));
    transform *= XMMatrixRotationX(XMConvertToRadians(XMVectorGetX(rotation)));
    transform *= XMMatrixRotationY(XMConvertToRadians(XMVectorGetY(rotation)));
    transform *= XMMatrixTranslationFromVector(position);
    return transform;
}
