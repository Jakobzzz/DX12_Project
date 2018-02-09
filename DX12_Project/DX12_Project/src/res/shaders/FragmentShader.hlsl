Texture2D t1 : register(t0);
Texture2D t2 : register(t1);
SamplerState s1 : register(s0);

cbuffer colorBuffer : register(b0)
{
    float4 color;
};

struct PixelShaderInput
{
    float4 pos : SV_Position;
	float2 uv : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return t1.Sample(s1, input.uv) * color;
}