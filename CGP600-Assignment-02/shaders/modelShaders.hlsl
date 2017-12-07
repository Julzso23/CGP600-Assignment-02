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
    float4 worldPosition : POSITION;
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
    output.worldPosition = input.position;
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

    float pointDiffuse = 0.f;
    float4 pointLightDirection = pointLightPosition - input.worldPosition;
    pointDiffuse = dot(normalize(pointLightDirection.xyz), normal);
    pointDiffuse = saturate(pointDiffuse);
    pointDiffuse *= 1.f - saturate(length(pointLightDirection) / pointLightFalloff);
    pointDiffuse = pow(pointDiffuse, 2.f);

    float directionalDiffuse = 0.f;
    directionalDiffuse += dot(normalize(lightDirection.xyz), normal);
    directionalDiffuse = saturate(directionalDiffuse);

    input.colour += pointDiffuse * pointLightColour;
    input.colour += directionalDiffuse * directionalLightColour;
    return input.colour * textures[0].Sample(sampler0, input.texcoord);
}
