//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct VS_IN
{
    float3 pos : POSITION;
};

struct VS_OUT
{
    float3 pos : POSITION;
};

struct GS_OUT
{
    float4 pos : SV_Position;
};

cbuffer matrixBuffer : register(b0)
{
   float4x4 WVP;
};

cbuffer cbImmutable
{
    float4 g_positions[4] =
    {
        float4(0.5f, -0.5f, 0.f, 0.f),
        float4(0.5f, 0.5f, 0.f, 0.f),
        float4(-0.5f, -0.5f, 0.f, 0.f),
        float4(-0.5f, 0.5f, 0.f, 0.f),
    };
}

cbuffer cbPointSize
{
    static float g_pointSize = 1.f;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;

    output.pos = input.pos;
    return output;
}

//--------------------------------------------------------------------------------------
// Geometry shader
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void GS_MAIN(point VS_OUT input[1], inout TriangleStream<GS_OUT> SpriteStream)
{
    GS_OUT output;
    
    float4 v[4];
    v[0] = float4(input[0].pos, 1.f) + g_positions[0];
    v[1] = float4(input[0].pos, 1.f) + g_positions[1];
    v[2] = float4(input[0].pos, 1.f) + g_positions[2];
    v[3] = float4(input[0].pos, 1.f) + g_positions[3];

    //Emit two new triangles
	[unroll]
    for (uint i = 0; i < 4; ++i)
    {
        output.pos = mul(v[i], WVP);
        SpriteStream.Append(output);
    }
    SpriteStream.RestartStrip();
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS_MAIN(GS_OUT input) : SV_TARGET
{
    return float4(1.f, 0.f, 0.f, 1.f);
}
