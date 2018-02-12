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

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    output.pos = mul(float4(input.pos, 1.f), WVP);
    output.uv = input.uv;
    return output;
}