//--------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------
struct BodyData
{
    float4 pos;
    float4 velocity;
};

StructuredBuffer<BodyData> g_particles : register(t0);

//--------------------------------------------------------------------------------------
// Global constants
//--------------------------------------------------------------------------------------
cbuffer cbDraw : register(b0)
{
    float4x4 g_mWorldViewProjection;
};

cbuffer cbImmutable
{
    float4 g_positions[4]  =
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

//--------------------------------------------------------------------------------------
// Vertex shader and pixel shader input/output structures
//--------------------------------------------------------------------------------------
struct DisplayVS_OUTPUT
{
    float4 Position : SV_POSITION;  
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
DisplayVS_OUTPUT VS_MAIN(uint id : SV_VERTEXID)
{
    DisplayVS_OUTPUT output;
    
    output.Position = mul(g_particles[id].pos, g_mWorldViewProjection);  
    return output;    
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS_MAIN(DisplayVS_OUTPUT In) : SV_TARGET
{ 
    return float4(0.3f, 1.f, 0.2f, 1.f);
}

//--------------------------------------------------------------------------------------
// Geometry shader for creating point sprites
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void GS_MAIN(point DisplayVS_OUTPUT input[1], inout TriangleStream<DisplayVS_OUTPUT> SpriteStream)
{
    DisplayVS_OUTPUT output;
    
    //Emit two new triangles
	[unroll]
    for (uint i = 0; i < 4; ++i)
    {
        output.Position = input[0].Position + float4(g_positions[i].xy * g_pointSize, 0.f, 0.f);
        SpriteStream.Append(output);
    }
    SpriteStream.RestartStrip();
}


