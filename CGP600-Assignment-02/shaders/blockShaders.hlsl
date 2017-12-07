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
    VOut output;
    output.position = mul(worldViewProjection, float4(input.position.xyz + input.instancePosition.xyz, 1.f));
    output.worldPosition = float4(input.position.xyz + input.instancePosition.xyz, 1.f);
    output.colour = ambientLightColour;
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
    normal = (normal.x * input.tangent.xyz) + (normal.y * input.binormal.xyz) + (normal.z * input.normal.xyz);

    float pointDiffuse = 0.f;
    float4 pointLightDirection = pointLightPosition - input.worldPosition;
    if (length(pointLightDirection) < 20.f)
    {
        pointDiffuse = dot(normalize(pointLightDirection.xyz), normal);
    }
    pointDiffuse = saturate(pointDiffuse);

    float directionalDiffuse = 0.f;
    directionalDiffuse += dot(normalize(lightDirection.xyz), normal);
    directionalDiffuse = saturate(directionalDiffuse);

    input.colour += pointDiffuse * pointLightColour;
    //input.colour += directionalDiffuse * directionalLightColour;
    return input.colour * textureColour;
}
