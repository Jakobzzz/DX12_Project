struct Color
{
    float4 color;
};

RWStructuredBuffer<Color> OutputBuffer : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    //Set the structured buffer to a green color
    OutputBuffer[DispatchThreadID.x].color = float4(0.f, 1.f, 0.f, 1.f);
}