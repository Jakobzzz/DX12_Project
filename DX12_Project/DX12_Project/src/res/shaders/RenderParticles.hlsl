//--------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------
struct BodyData
{
    float4 pos;
    float4 velocity;
};

Texture2D<float4> g_ParticleTex : register(t1);
StructuredBuffer<BodyData> g_particles : register(t0);

SamplerState g_particleSampler : register(s0);

//--------------------------------------------------------------------------------------
// Global constants
//--------------------------------------------------------------------------------------
cbuffer cbDraw : register(b0)
{
    row_major float4x4 g_mWorldViewProjection;
};

cbuffer cbImmutable
{
    static float4 g_positions[4] =
    {
        float4(0.5f, -0.5f, 0.f, 0.f),
        float4(0.5f, 0.5f, 0.f, 0.f),
        float4(-0.5f, -0.5f, 0.f, 0.f),
        float4(-0.5f, 0.5f, 0.f, 0.f),
    };

    static float4 g_texcoords[4] =
    {
        float4(1.f, 0.f, 0.f, 0.f),
        float4(1.f, 1.f, 0.f, 0.f),
        float4(0.f, 0.f, 0.f, 0.f),
        float4(0.f, 1.f, 0.f, 0.f),
    };
}

cbuffer cbPointSize
{
    static float g_pointSize = 1.0f;
};

//--------------------------------------------------------------------------------------
// Vertex shader and pixel shader input/output structures
//--------------------------------------------------------------------------------------
struct VS_OUT
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
};

struct GS_OUT
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUT VS_MAIN(uint id : SV_VERTEXID)
{
    VS_OUT output;
    
    output.position = mul(g_particles[id].pos, g_mWorldViewProjection);
    output.uv = float2(0.f, 0.f);
    return output;    
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS_MAIN(GS_OUT input) : SV_TARGET
{ 
    float tex = g_ParticleTex.Sample(g_particleSampler, input.uv).x;
    
    if (tex.x < .05f) 
        discard;
	
    return tex.xxxx * float4(0.3f, 1.f, 0.2f, 1.f);
}

//--------------------------------------------------------------------------------------
// Geometry shader for creating point sprites
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void GS_MAIN(point VS_OUT input[1], inout TriangleStream<GS_OUT> SpriteStream)
{
    GS_OUT output;
    
    //Emit two new triangles
	[unroll]
    for (uint i = 0; i < 4; ++i)
    {
        output.position = input[0].position + float4(g_positions[i].xy * g_pointSize, 0.f, 0.f);
        output.uv = g_texcoords[i];
        SpriteStream.Append(output);
    }
    SpriteStream.RestartStrip();
}


