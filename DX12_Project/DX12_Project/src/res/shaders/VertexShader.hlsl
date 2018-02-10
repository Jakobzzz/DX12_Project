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

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    output.pos = float4(input.pos, 1.f);
    output.uv = input.uv;
    return output;
}