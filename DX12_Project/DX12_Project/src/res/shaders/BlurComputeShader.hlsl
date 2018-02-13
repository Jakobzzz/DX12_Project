//Input and output resources
RWStructuredBuffer<float4> OutputBuffer : register(u0);

//Group size
#define size_x 32
#define size_y 32

//Declare one thread for each texel of the current block
[numthreads(size_x, size_y, 1)]

void main(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    //Fill the output buffer with a red color
    OutputBuffer[DispatchThreadID.x] = float4(1.f, 0.f, 0.f, 1.f);
}