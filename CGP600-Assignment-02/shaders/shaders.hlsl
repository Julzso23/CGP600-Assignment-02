cbuffer CBuffer0
{
    matrix worldViewProjection;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD)
{
    VOut output;
    output.position = mul(worldViewProjection, position);
    output.color = color;
    output.texcoord = texcoord;
    return output;
}

Texture2D texture0;
SamplerState sampler0;

float4 PShader(float4 position : SV_POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
    return color * texture0.Sample(sampler0, texcoord);
}
