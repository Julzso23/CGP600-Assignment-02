struct Block
{
    bool shouldCollide;
    float4 position;
    float4 size;
};

StructuredBuffer<Block> Buffer0 : register(t0);

[numthreads(10, 10, 10)]
void CShader(uint3 dispatchThreadID : SV_DispatchThreadID)
{

}