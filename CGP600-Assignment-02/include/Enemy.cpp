#include "Enemy.hpp"

Mesh* Enemy::getMesh()
{
    return &mesh;
}

void Enemy::draw(ID3D11DeviceContext* immediateContext)
{
    ID3D11ShaderResourceView* textures[] = {
        mesh.getTexture(),
        mesh.getNormalMap()
    };

    UINT vertexCount;
    ID3D11Buffer* vertexBuffer = mesh.getVertexBuffer(&vertexCount);

    immediateContext->PSSetShaderResources(0, 2, textures);
    immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, 0, 0);
    immediateContext->Draw(vertexCount, 0);
}
