cbuffer CBuffer0
{
    matrix worldViewProjection;
    float4 lightDirection;
    float4 lightColour;
    float4 ambientLightColour;
};

struct VIn
{
    float4 position : POSITION;
    float4 colour : COLOR;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 instancePosition : TEXCOORD1;
    uint textureId : TEXCOORD2;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 colour : COLOR;
    float2 texcoord : TEXCOORD;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    uint textureId : TEXCOORD2;
};

Texture2D texture0;
SamplerState sampler0;

VOut VShader(VIn input)
{
    VOut output;
    output.position = mul(worldViewProjection, float4(input.position.xyz + input.instancePosition, 1.f));
    output.colour = ambientLightColour;
    output.texcoord = input.texcoord / 2.f;
	output.normal = input.normal;
    output.tangent = input.tangent;
    output.binormal = input.binormal;
    output.textureId = input.textureId;
    return output;
}

float4 PShader(VOut input) : SV_TARGET
{
    float3 normal = (texture0.Sample(sampler0, input.texcoord + float2(0.5f, input.textureId / 2.f)).xyz * 2.f) - 1.f;
    normal = (normal.x * input.tangent) + (normal.y * input.binormal) + (normal.z * input.normal);
    float diffuseAmount = dot(lightDirection.xyz, normal);
    diffuseAmount = saturate(diffuseAmount);

    input.colour += diffuseAmount * lightColour;
    return input.colour * texture0.Sample(sampler0, input.texcoord + float2(0.f, input.textureId / 2.f));
}
