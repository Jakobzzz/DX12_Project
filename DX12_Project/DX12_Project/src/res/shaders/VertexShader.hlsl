//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_OUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

cbuffer matrixBuffer : register(b0)
{
    float4x4 WVP;
};

VS_OUT VS_MAIN(VS_IN input)
{
    VS_OUT output;

    output.pos = mul(float4(input.pos, 1.f), WVP);
    output.uv = input.uv;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
SamplerState s1 : register(s0);

struct Color
{
    float4 color;
};

StructuredBuffer<Color> colorBuffer : register(t0); // SRV

float4 PS_MAIN(VS_OUT input) : SV_TARGET
{
    return colorBuffer[0].color;
}