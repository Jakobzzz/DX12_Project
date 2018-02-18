//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

//Texture2D <float4>g_ParticleTex : TEXTURE0;
StructuredBuffer<float4> g_particles : register(t0);

//--------------------------------------------------------------------------------------
// Global constants
//--------------------------------------------------------------------------------------
cbuffer cbDraw : register(b0)
{
    float4x4 g_mWorldViewProjection;
    float	 g_fPointSize;
    uint     g_readOffset;
};

cbuffer cbImmutable : register(b1)
{
    float4 g_positions[4];
    //float4 g_texcoords[4];
}

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------
//SamplerState LinearSampler
//{
//    Filter   = MIN_MAG_MIP_LINEAR;
//    AddressU = Clamp;
//    AddressV = Clamp;
//};

//--------------------------------------------------------------------------------------
// Vertex shader and pixel shader input/output structures
//--------------------------------------------------------------------------------------


struct VS_Input_Indices
{
    uint pId : SV_VertexID;   
};

struct DisplayVS_OUTPUT
{
    float4 Position : SV_POSITION;   // vertex position 
    //float2 uv : TEXCOORD0;
    float  PointSize : PSIZE;		   // point size;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
DisplayVS_OUTPUT VS_MAIN( VS_Input_Indices In)
{
    DisplayVS_OUTPUT Output = (DisplayVS_OUTPUT) 0;
    
    float4 pos = g_particles[g_readOffset + In.pId];
    
     // Transform the position from object space to homogeneous projection space
    Output.Position  = mul(pos, g_mWorldViewProjection);
    Output.PointSize = g_fPointSize;
    //Output.uv = float2(0.f, 0.f);
         
    return Output;    
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS_MAIN(DisplayVS_OUTPUT In) : SV_TARGET
{ 
    return float4(0.3f, 1.f, 0.2f, 1.f);
    //float tex = g_ParticleTex.Sample(LinearSampler, In.uv).x;
	
    //if (tex.x < .05) 
    //    discard;
	
    //Output.RGBColor = tex.xxxx * float4(0.3, 1.0, 0.2, 1.0);
    //return Output;
}

//--------------------------------------------------------------------------------------
//
// Geometry shader for creating point sprites
//
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void GS_MAIN(point DisplayVS_OUTPUT input[1], inout TriangleStream<DisplayVS_OUTPUT> SpriteStream)
{
    DisplayVS_OUTPUT output;
    
    //Emit two new triangles
	[unroll]
    for (uint i = 0; i < 4; ++i)
    {
        output.Position = input[0].Position + float4(g_positions[i].xy * input[0].PointSize, 0, 0);
        //output.uv = g_texcoords[i];
        output.PointSize = input[0].PointSize;
		
        SpriteStream.Append(output);
    }
    SpriteStream.RestartStrip();
}


