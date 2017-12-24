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
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 colour : COLOR;
    float2 texcoord : TEXCOORD;
};

Texture2D textures[2];
SamplerState sampler0;

VOut VShader(VIn input)
{
    VOut output;
    output.position = mul(worldViewProjection, input.position);
    output.colour = input.colour;
    output.texcoord = input.texcoord;
    return output;
}

float4 PShader(VOut input) : SV_TARGET
{
    return input.colour * textures[0].Sample(sampler0, input.texcoord);
}
