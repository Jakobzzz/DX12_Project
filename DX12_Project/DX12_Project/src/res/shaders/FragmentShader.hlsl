SamplerState s1 : register(s0);

struct Color
{
    float4 color;
};

StructuredBuffer<Color> colorBuffer : register(t0); // SRV

struct PS_IN
{
    float4 pos : SV_Position;
	float2 uv : TEXCOORD;
};

float4 main(PS_IN input) : SV_TARGET
{
    return colorBuffer[0].color;
}