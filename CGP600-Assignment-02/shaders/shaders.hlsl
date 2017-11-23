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
    float3 instancePosition : INST_POSITION;
    uint textureId : TEXID;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 colour : COLOR;
    float2 texcoord : TEXCOORD;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    uint textureId : TEXID;
};

Texture2D textures[4];
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
    float4 textureColour;
    float4 normalColour;

    if (input.textureId == 0)
    {
        textureColour = textures[0].Sample(sampler0, input.texcoord);
        normalColour = textures[1].Sample(sampler0, input.texcoord);
    }
    else
    {
        textureColour = textures[2].Sample(sampler0, input.texcoord);
        normalColour = textures[3].Sample(sampler0, input.texcoord);
    }
    
    float3 normal = (normalColour.xyz * 2.f) - 1.f;
    normal = (normal.x * input.tangent) + (normal.y * input.binormal) + (normal.z * input.normal);
    float diffuseAmount = dot(lightDirection.xyz, normal);
    diffuseAmount = saturate(diffuseAmount);

    input.colour += diffuseAmount * lightColour;
    return input.colour * textureColour;
}
