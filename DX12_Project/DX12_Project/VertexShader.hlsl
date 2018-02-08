struct VertexShaderInput
{
    float3 pos : POSITION;
	float2 uv : TEXCOORD;
};

struct VertexShaderOutput
{
    float3 pos : SV_Position;
	float2 uv : TEXCOORD;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;	

    output.pos = input.pos;
	output.uv = input.uv;
	return output;
}