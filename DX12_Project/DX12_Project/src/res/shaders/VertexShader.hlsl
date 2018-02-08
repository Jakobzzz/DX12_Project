struct VertexShaderInput
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    output.pos = float4(input.pos, 1.f);
    output.uv = input.uv;
    return output;
}