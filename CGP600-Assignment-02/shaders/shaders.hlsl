cbuffer CBuffer0
{
    matrix worldViewProjection;
    float4 lightDirection;
    float4 lightColour;
    float4 ambientLightColour;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
	float3 normal : NORMAL;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD, float3 normal : NORMAL)
{
    VOut output;
    output.position = mul(worldViewProjection, float4(position.xyz, 1.f));
    float diffuseAmount = dot(lightDirection, normal);
    diffuseAmount = saturate(diffuseAmount);
    output.color = ambientLightColour + (lightColour * diffuseAmount);
    output.texcoord = texcoord;
	output.normal = normal;
    return output;
}

Texture2D texture0;
SamplerState sampler0;

float4 PShader(float4 position : SV_POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD, float3 normal : NORMAL) : SV_TARGET
{
    return color * texture0.Sample(sampler0, texcoord);
}
