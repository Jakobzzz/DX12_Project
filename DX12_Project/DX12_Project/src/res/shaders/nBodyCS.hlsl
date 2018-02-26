#define BLOCK_SIZE 256

//Constants used by the compute shader
cbuffer cbUpdate : register(b0)
{
	float g_timestep;
	float g_softeningSquared;
	uint  g_numParticles;
    uint g_numBlocks;
};	

struct BodyData
{
    float4 pos;
    float4 velocity;
};

RWStructuredBuffer<BodyData> particles : register(u0);

// This function computes the gravitational attraction between two bodies
// at positions bi and bj. The mass of the bodies is stored in the w 
// component
float3 BodyBodyInteraction(float4 bi, float4 bj, int particles) 
{
    float3 r = bi - bj;

    float distSqr = dot(r, r);
    distSqr += g_softeningSquared;

    float invDist = 1.0f / sqrt(distSqr);
	float invDistCube =  invDist * invDist * invDist;

    float s = bj.w * invDistCube * particles;

    return r * s;
}

// This groupshared memory is used to cache BLOCK_SIZE body positions
// in on-chip shared memory so that we can achieve maximal reuse of 
// data loaded from global memory
groupshared float4 sharedPos[BLOCK_SIZE];

// The main gravitation function, computes the interaction between
// a body and all other bodies in the system
float3 Gravitation(float4 myPos, float3 accel)
{
    uint i = 0;

    [unroll]
    for (uint counter = 0; counter < BLOCK_SIZE; counter++) 
        accel += BodyBodyInteraction(sharedPos[i++], myPos, 1); 

    // g_numParticles is the number of our particles, however this number might not 
	// be an exact multiple of the tile size. In such cases, out of bound reads 
	// occur in the process above, which means there will be tooManyParticles 
	// "phantom" particles generating false gravity at position (0, 0, 0), so 
	// we have to subtract them here. NOTE, out of bound reads always return 0 in CS.
    const int tooManyParticles = g_numBlocks * BLOCK_SIZE - g_numParticles;

    accel += BodyBodyInteraction(float4(0.0f, 0.0f, 0.0f, 0.0f), myPos, -tooManyParticles);

    return accel;
}

// Computes the total acceleration on the body with position myPos 
// caused by the gravitational attraction of all other bodies in 
// the simulation
float3 ComputeBodyAccel(float4 bodyPos, uint threadId, uint blockId)
{
    float3 acceleration = {0.0f, 0.0f, 0.0f};
    uint p = BLOCK_SIZE;
    uint n = g_numParticles;
    uint numTiles = n / p;

    for (uint tile = 0; tile < numTiles; tile++) 
    {
        sharedPos[threadId] = particles[tile * p + threadId].pos;
       
        GroupMemoryBarrierWithGroupSync();
        acceleration = Gravitation(bodyPos, acceleration);
        GroupMemoryBarrierWithGroupSync();
    }

    return acceleration;
}

// NBodyUpdate is the compute shader entry point for the n-body simulation
// This function first computes the acceleration on all bodies in parallel,
// and then integrates the velocity and position to get the new state of
// all particles, using a simple Leapfrog-Verlet integration step.
[numthreads(BLOCK_SIZE, 1 ,1)]
void CS_MAIN(uint threadId : SV_GroupIndex, uint3 groupId : SV_GroupID, uint3 globalThreadId : SV_DispatchThreadID)
{	
    float4 pos = particles[globalThreadId.x].pos; 
    float4 vel = particles[globalThreadId.x].velocity;  

	//Compute acceleration
	float3 accel = ComputeBodyAccel(pos, threadId, groupId.x);
	
	//Leapfrog-Verlet integration of velocity and position
	vel.xyz += accel * g_timestep;
	pos.xyz += vel * g_timestep;
    
	particles[globalThreadId.x].pos = pos;
	particles[globalThreadId.x].velocity = vel;
}
