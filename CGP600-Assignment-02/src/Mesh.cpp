#include "Mesh.hpp"
#include "Utility.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <Windows.h>
#include <WICTextureLoader.h>
#include <d3dcompiler.h>

using namespace DirectX;

HRESULT Mesh::initialiseVertexBuffer(ID3D11Device* device, ID3D11DeviceContext* immediateContext)
{
	std::lock_guard<std::mutex> lock(mutex);

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
	std::lock_guard<std::mutex> lock(mutex);

    *vertexCount = (UINT)vertices.size();
    return vertexBuffer;
}

HRESULT Mesh::loadTexture(ID3D11Device* device, const wchar_t* fileName, const wchar_t* normalMapFileName)
{
	std::lock_guard<std::mutex> lock(mutex);

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
	std::lock_guard<std::mutex> lock(mutex);
    return texture;
}

ID3D11ShaderResourceView* Mesh::getNormalMap() const
{
	std::lock_guard<std::mutex> lock(mutex);
    return normalMap;
}

void Mesh::loadShaders(LPWSTR path, ID3D11Device* device, D3D11_INPUT_ELEMENT_DESC* inputElementDescriptions, UINT descriptionCount)
{
	std::lock_guard<std::mutex> lock(mutex);

    ID3DBlob* vertShader = nullptr;
    ID3DBlob* pixShader = nullptr;
    ID3DBlob* error = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif

    // Vertex shader compile
    HRESULT result = D3DCompileFromFile(path, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VShader", "vs_5_0", flags, NULL, &vertShader, &error);
    if (error != 0)
    {
        OutputDebugString((char*)error->GetBufferPointer());
        error->Release();
        if (FAILED(result))
        {
            OutputDebugString("#### Failed to compile vertex shader! ####\n");
            //return result;
        }
    }

    // Pixel shader compile
    result = D3DCompileFromFile(path, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PShader", "ps_5_0", flags, NULL, &pixShader, &error);
    if (error != 0)
    {
        OutputDebugString((char*)error->GetBufferPointer());
        error->Release();
        if (FAILED(result))
        {
            OutputDebugString("#### Failed to compile pixel shader! ####\n");
            //return result;
        }
    }

    // Vertex shader create
    result = device->CreateVertexShader(vertShader->GetBufferPointer(), vertShader->GetBufferSize(), NULL, &vertexShader);
    if (FAILED(result))
    {
        OutputDebugString("#### Failed to create vertex shader! ####\n");
        //return result;
    }

    // Pixel shader create
    result = device->CreatePixelShader(pixShader->GetBufferPointer(), pixShader->GetBufferSize(), NULL, &pixelShader);
    if (FAILED(result))
    {
        OutputDebugString("#### Failed to create pixel shader! ####\n");
        //return result;
    }

    result = device->CreateInputLayout(
        inputElementDescriptions,
        descriptionCount,
        vertShader->GetBufferPointer(),
        vertShader->GetBufferSize(),
        &inputLayout
    );

    if (FAILED(result))
    {
        OutputDebugString("#### Failed to create input layout! ####\n");
        //return result;
    }

    D3D11_SAMPLER_DESC samplerDescription;
    ZeroMemory(&samplerDescription, sizeof(samplerDescription));
    samplerDescription.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

    result = device->CreateSamplerState(&samplerDescription, &sampler0);

    if (FAILED(result))
    {
        OutputDebugString("#### Failed to create sampler state! ####");
        //return result;
    }
}

void Mesh::setShaders(ID3D11DeviceContext* immediateContext)
{
    immediateContext->IASetInputLayout(inputLayout);
    immediateContext->VSSetShader(vertexShader, 0, 0);
    immediateContext->PSSetShader(pixelShader, 0, 0);
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    immediateContext->PSSetSamplers(0, 1, &sampler0);
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
    if (sampler0) sampler0->Release();
    if (inputLayout) inputLayout->Release();
    if (vertexShader) vertexShader->Release();
    if (pixelShader) pixelShader->Release();
}

static void float4Normalize(XMFLOAT4* value)
{
    XMStoreFloat4(value, XMVector4Normalize(XMLoadFloat4(value)));
}

void Mesh::loadFromFile(const char* fileName)
{
	std::lock_guard<std::mutex> lock(mutex);

    std::ifstream file(fileName);

    if (!file)
    {
        OutputDebugString((std::string("#### Failed to load file: ") + std::string(fileName) + " ####\n").c_str());
    }

    std::vector<Vertex> newVertices;
    std::vector<XMFLOAT2> textureCoords;
	std::vector<XMFLOAT4> normals;

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
			normals.push_back(XMFLOAT4((float)std::atof(data[1].c_str()), (float)std::atof(data[2].c_str()), (float)std::atof(data[3].c_str()), 0.f));

			continue;
		}

        if (data[0] == "f")
        {
            std::vector<Vertex> face(3);

            for (int i = 1; i <= 3; i++)
            {
                std::vector<std::string> vertexData = Utility::split(data[i], '/');

                int vertexIndex = std::atoi(vertexData[0].c_str());
                indices.push_back(vertexIndex);
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
            XMFLOAT4 tangent = {
                ((vVector.y * vector0.x) - (vVector.x * vector1.x)) * denominator,
                ((vVector.y * vector0.y) - (vVector.x * vector1.y)) * denominator,
                ((vVector.y * vector0.z) - (vVector.x * vector1.z)) * denominator,
                0.f
            };

            XMFLOAT4 binormal = {
                ((uVector.x * vector1.x) - (uVector.y * vector0.x)) * denominator,
                ((uVector.x * vector1.y) - (uVector.y * vector0.y)) * denominator,
                ((uVector.x * vector1.z) - (uVector.y * vector0.z)) * denominator,
                0.f
            };

            float4Normalize(&tangent);
            float4Normalize(&binormal);

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
	std::lock_guard<std::mutex> lock(mutex);
    return &vertices;
}

void Mesh::setPosition(XMVECTOR position)
{
	std::lock_guard<std::mutex> lock(mutex);
    this->position = position;
}

XMVECTOR Mesh::getPosition() const
{
	std::lock_guard<std::mutex> lock(mutex);
    return position;
}

void Mesh::setRotation(XMVECTOR rotation)
{
	std::lock_guard<std::mutex> lock(mutex);
    this->rotation = rotation;
}

XMVECTOR Mesh::getRotation() const
{
	std::lock_guard<std::mutex> lock(mutex);
    return rotation;
}

void Mesh::setScale(XMVECTOR scale)
{
	std::lock_guard<std::mutex> lock(mutex);
    this->scale = scale;
}

XMVECTOR Mesh::getScale() const
{
	std::lock_guard<std::mutex> lock(mutex);
    return scale;
}

XMMATRIX Mesh::getTransform() const
{
	std::lock_guard<std::mutex> lock(mutex);

    XMMATRIX transform = XMMatrixIdentity();
    transform *= XMMatrixScalingFromVector(scale);
    transform *= XMMatrixRotationZ(XMConvertToRadians(XMVectorGetZ(rotation)));
    transform *= XMMatrixRotationX(XMConvertToRadians(XMVectorGetX(rotation)));
    transform *= XMMatrixRotationY(XMConvertToRadians(XMVectorGetY(rotation)));
    transform *= XMMatrixTranslationFromVector(position);
    return transform;
}
