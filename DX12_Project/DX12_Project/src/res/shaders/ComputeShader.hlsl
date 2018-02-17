struct Color
{
    float4 color;
};

StructuredBuffer<Color> inputBuffer : register(t0);
RWStructuredBuffer<Color> OutputBuffer : register(u0);

[numthreads(1, 1, 1)]
void CS_MAIN(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    if (inputBuffer[DispatchThreadID.x].color.y > 1.0f)
        OutputBuffer[DispatchThreadID.x].color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    else
        OutputBuffer[DispatchThreadID.x].color = (inputBuffer[DispatchThreadID.x].color + float4(0.0f, 0.1f, 0.0f, 1.0f));
}