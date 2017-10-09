cbuffer CBuffer0
{
    float redAmount;
    float scale;
    float2 packing;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR)
{
    color.r *= redAmount;

    position.x *= scale;
    position.y *= scale;
    position.z *= scale;

    VOut output;
    output.position = position;
    output.color = color;
    return output;
}

float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return color;
}
