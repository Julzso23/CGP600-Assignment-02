RWBuffer<uint> blockValues : register(u0);
RWStructuredBuffer<int> permutation : register(u1);

static const int width = 64;
static const int height = 64;
static const int depth = 64;

float fade(float time)
{
    return time * time * time * (time * (time * 6.f - 15.f) + 10.f);
}

float lerp(float time, float a, float b)
{
    return a + time * (b - a);
}

float gradient(int hash, float x, float y, float z)
{
    int Hash = hash & 15;
    float u = Hash < 8 ? x : y;
    float v = Hash < 4 ? y : (Hash == 12 || Hash == 14 ? x : z);
    return ((Hash & 1) == 0 ? u : -u) + ((Hash & 2) == 0 ? v : -v);
}

float noise(int x, int y, int z)
{
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    int Z = (int)floor(z) & 255;

    x -= floor(x);
    y -= floor(y);
    z -= floor(z);

    float u = fade(x);
    float v = fade(y);
    float w = fade(z);

    int A = permutation[X] + Y;
    int AA = permutation[A] + Z;
    int AB = permutation[A + 1] + Z;
    int B = permutation[X + 1] + Y;
    int BA = permutation[B] + Z;
    int BB = permutation[B + 1] + Z;

    float result = lerp(
        w,
        lerp(
            v,
            lerp(u, gradient(permutation[AA], x, y, z), gradient(permutation[BA], x - 1, y, z)),
            lerp(u, gradient(permutation[AB], x, y - 1, z), gradient(permutation[BB], x - 1, y - 1, z))
        ),
        lerp(
            v,
            lerp(u, gradient(permutation[AA + 1], x, y, z - 1), gradient(permutation[BA + 1], x - 1, y, z - 1)),
            lerp(u, gradient(permutation[AB + 1], x, y - 1, z - 1), gradient(permutation[BB + 1], x - 1, y - 1, z - 1))
        )
    );

    return (result + 1.f) / 2.f;
}

int getBlockIndex(int x, int y, int z)
{
    return x + width * (y + height * z);
}

[numthreads(8, 8, 8)]
void CShader(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    for (int x = dispatchThreadID.x * 8; x < width / 8; x++)
    {
        for (int y = dispatchThreadID.y * 8; y < height / 8; y++)
        {
            for (int z = dispatchThreadID.z * 8; z < depth / 8; z++)
            {
                //if (noise(x, y, z) > 0.5f)
                {
                    blockValues[getBlockIndex(x, y, z)] = 9999;
                }
                /*else
                {
                    blockValues[getBlockIndex(x, y, z)] = false;
                }*/
            }
        }
    }
}
