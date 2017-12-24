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
    float directionalDiffuse = dot(normalize(lightDirection), input.normal);
    directionalDiffuse = saturate(directionalDiffuse);

    VOut output;
    output.position = mul(worldViewProjection, input.position);
    output.worldPosition = input.position;
    output.colour = ambientLightColour + (directionalDiffuse * directionalLightColour);
    output.texcoord = input.texcoord;
	output.normal = input.normal;
    output.tangent = input.tangent;
    output.binormal = input.binormal;
    return output;
}

float4 PShader(VOut input) : SV_TARGET
{
    float3 normal = (textures[1].Sample(sampler0, input.texcoord).xyz * 2.f) - 1.f;
    normal = (normal.x * input.tangent) + (normal.y * input.binormal) + (normal.z * input.normal);

    float4 pointLightDirection = pointLightPosition - input.worldPosition;
    float pointDiffuse = dot(normalize(pointLightDirection.xyz), normal);
    pointDiffuse = saturate(pointDiffuse);
    pointDiffuse *= 1.f - saturate(length(pointLightDirection) / pointLightFalloff);
    pointDiffuse = pow(pointDiffuse, 2.f);

    input.colour += pointDiffuse * pointLightColour;
    return input.colour * textures[0].Sample(sampler0, input.texcoord);
}
