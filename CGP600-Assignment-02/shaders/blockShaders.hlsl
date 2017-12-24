cbuffer VertexConstantBuffer
{
    matrix worldViewProjection;
    float4 ambientLightColour;
    float4 lightDirection;
    float4 directionalLightColour;
};

cbuffer PixelConstantBuffer
{
    float4 pointLightPosition;
    float4 pointLightColour;
    float pointLightFalloff;
    float3 padding;
};

struct VIn
{
    float4 position : POSITION;
    float4 colour : COLOR;
    float2 texcoord : TEXCOORD;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 binormal : BINORMAL;
    float4 instancePosition : INST_POSITION;
    uint textureId : TEXID;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
    float4 colour : COLOR;
    float2 texcoord : TEXCOORD;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 binormal : BINORMAL;
    uint textureId : TEXID;
};

Texture2D textures[4];
SamplerState sampler0;

VOut VShader(VIn input)
{
    float directionalDiffuse = dot(normalize(lightDirection), input.normal);
    directionalDiffuse = saturate(directionalDiffuse);

    VOut output;
    output.position = mul(worldViewProjection, float4(input.position.xyz + input.instancePosition.xyz, 1.f));
    output.worldPosition = float4(input.position.xyz + input.instancePosition.xyz, 1.f);
    output.colour = ambientLightColour + (directionalDiffuse * directionalLightColour);
    output.texcoord = input.texcoord;
	output.normal = input.normal;
    output.tangent = input.tangent;
    output.binormal = input.binormal;
    output.textureId = input.textureId;
    return output;
}

float4 PShader(VOut input) : SV_TARGET
{
    float4 textureColour;
    float4 normalColour;

    switch (input.textureId)
    {
        default:
        case 0:
        {
            textureColour = textures[0].Sample(sampler0, input.texcoord);
            normalColour = textures[1].Sample(sampler0, input.texcoord);
            break;
        }
        case 1:
        {
            textureColour = textures[2].Sample(sampler0, input.texcoord);
            normalColour = textures[3].Sample(sampler0, input.texcoord);
            break;
        }
    }
    
    float3 normal = (normalColour.xyz * 2.f) - 1.f;
    float3x3 TBN = float3x3(input.tangent.xyz, input.binormal.xyz, input.normal.xyz);
    TBN = transpose(TBN);
    normal = mul(TBN, normal);

    float4 pointLightDirection = pointLightPosition - input.worldPosition;
    float pointDiffuse = dot(normalize(pointLightDirection.xyz), normal);
    pointDiffuse = saturate(pointDiffuse);
    pointDiffuse *= 1.f - saturate(length(pointLightDirection) / pointLightFalloff);
    pointDiffuse = pow(pointDiffuse, 2.f);

    input.colour += pointDiffuse * pointLightColour * 2.f;
    return input.colour * textureColour;
}
