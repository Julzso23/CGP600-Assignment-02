cbuffer VertexConstantBuffer
{
    matrix worldViewProjection;
    float4 ambientLightColour;
};

cbuffer PixelConstantBuffer
{
    float4 lightDirection;
    float4 directionalLightColour;
    float4 pointLightPosition;
    float4 pointLightColour;
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
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

Texture2D textures[2];
SamplerState sampler0;

VOut VShader(VIn input)
{
    VOut output;
    output.position = mul(worldViewProjection, input.position);
    output.colour = ambientLightColour;
    output.texcoord = input.texcoord / 2.f;
	output.normal = input.normal;
    output.tangent = input.tangent;
    output.binormal = input.binormal;
    return output;
}

float4 PShader(VOut input) : SV_TARGET
{
    float3 normal = (textures[1].Sample(sampler0, input.texcoord).xyz * 2.f) - 1.f;
    normal = (normal.x * input.tangent) + (normal.y * input.binormal) + (normal.z * input.normal);
    float diffuseAmount = dot(lightDirection.xyz, normal);
    diffuseAmount = saturate(diffuseAmount);

    input.colour += diffuseAmount * directionalLightColour;
    return input.colour * textures[0].Sample(sampler0, input.texcoord);
}
