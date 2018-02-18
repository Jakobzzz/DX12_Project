#include <graphics/nbody/nBodySystemCS.hpp>

//Constant buffer for rendering particles
struct CB_DRAW
{
	Matrix g_mWorldViewProjection;
	float g_fPointSize;
    UINT g_readOffset;
	Vector2 padding;
};

//Constant buffer for the simulation update compute shader
struct CB_UPDATE
{
	float g_timestep;
    float g_softeningSquared;
	UINT g_numParticles;
    UINT g_readOffset;
    UINT g_writeOffset;
	Vector3 padding;
};

struct CB_IMMUTABLE
{
	Vector4 g_positions[4];
	//Vector4 g_texcoords[4];
};

//Hardcoded positions and texture-coordinates
CB_IMMUTABLE cbImmutable =
{
	Vector4(0.5f, -0.5f, 0.f, 0.f), Vector4(0.5f, 0.5f, 0.f, 0.f), Vector4(-0.5f, -0.5f, 0.f, 0.f), Vector4(-0.5f,  0.5f, 0.f, 0.f), //Positions
	//Vector4(1.f, 0.f, 0.f, 0.f), Vector4(1.f, 1.f, 0.f, 0.f), Vector4(0.f, 0.f, 0.f, 0.f), Vector4(0.f, 1.f, 0.f, 0.f), //Texcoords
};

//FLOAT blendFactors[] = { 1.0f, 1.0f, 1.0f, 1.0f };

namespace dx
{
	NBodySystemCS::NBodySystemCS(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, Buffer* buffer, Camera* camera) : m_device(device), 
								 m_commandList(commandList), m_buffer(buffer), m_camera(camera), m_readBuffer(0)
	{
	}

	//Create resources
	void NBodySystemCS::Initialize()
	{
		//Create constant buffers (two for normal pipeline and one for compute shader)
		m_buffer->CreateConstantBuffer(m_cbDrawUploadHeap->GetAddressOf(), &m_cbDrawAddress[0]);
		m_buffer->CreateConstantBuffer(m_cbImmutableUploadHeap->GetAddressOf(), &m_cbImmutableAddress[0]);
		m_buffer->CreateConstantBuffer(m_cbUpdateUploadHeap->GetAddressOf(), &m_cbUpdateAddress[0]);

		//Descriptor heap
		m_srvUavDescHeap = std::make_unique<DescriptorHeap>(m_device, m_commandList, 1);
		m_srvUavDescHeap->CreateDescriptorHeap(2, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	//Reset to initial state
	void NBodySystemCS::ResetBodies(BodyData* configData)
	{
		//Create SRV buffer for normal pipeline
		m_buffer->CreateSRVForRootTable(configData, sizeof(configData), sizeof(BodyData), NUM_BODIES, m_srvBuffer.GetAddressOf(),
			m_srvBufferUploadHeap.GetAddressOf(), m_srvUavDescHeap->GetCPUIncrementHandle(0));

		//Create UAV buffer for compute shader
		m_buffer->CreateUAVForRootTable(configData, sizeof(configData), sizeof(BodyData), NUM_BODIES, m_uavBuffer.GetAddressOf(),
			m_uavBufferUploadHeap.GetAddressOf(), m_srvUavDescHeap->GetCPUIncrementHandle(1));
	}

	// Render the bodies as particles using sprites
	void NBodySystemCS::RenderBodies(Shader* shader, RootSignature* signature, Matrix world, const UINT & frameIndex)
	{
		//Set constant buffer data for normal pipeline
		CB_DRAW cbDraw;
		//world = XMMatrixIdentity();
		Matrix WVP = world * m_camera->GetViewProjectionMatrix();
		cbDraw.g_mWorldViewProjection = XMMatrixTranspose(WVP);
		cbDraw.g_fPointSize = m_fPointSize;
		cbDraw.g_readOffset = m_readBuffer * NUM_BODIES; //Unsure about this

		m_buffer->SetConstantBufferData(&cbDraw, sizeof(cbDraw), frameIndex, &m_cbDrawAddress[0]);
		m_buffer->SetConstantBufferData(&cbImmutable, sizeof(cbImmutable), frameIndex, &m_cbImmutableAddress[0]);
		m_buffer->BindConstantBufferForRootDescriptor(0, frameIndex, m_cbDrawUploadHeap->GetAddressOf()); //Root index 0
		m_buffer->BindConstantBufferForRootDescriptor(1, frameIndex, m_cbImmutableUploadHeap->GetAddressOf()); //Root index 1

		//Set the normal NBody shader and root signature
		m_commandList->SetPipelineState(shader->GetShaders(Shaders::ID::NBody).pipelineState.Get());
		signature->SetRootSignature();
		m_srvUavDescHeap->SetComputeRootDescriptorTable(2, m_srvUavDescHeap->GetGPUIncrementHandle(0)); //Root index 2 for SRV
		shader->SetTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		//Draw particles
		m_commandList->DrawInstanced(NUM_BODIES, 1, 0, 0); //TODO: draw these as instanced?
	}

	//Update the positions and velocities of all bodies in the system
	void NBodySystemCS::UpdateBodies(Shader* shader, RootSignature* signature, float dt, const UINT & frameIndex)
	{
		//Set the time step
		dt = 0.016f;

		//Update the data for the compute constant buffer
		CB_UPDATE cbUpdate;
		cbUpdate.g_timestep = dt;
		cbUpdate.g_softeningSquared = 0.01f;
		cbUpdate.g_numParticles = NUM_BODIES;
		cbUpdate.g_readOffset = m_readBuffer * NUM_BODIES; //Unsure about this 
		cbUpdate.g_writeOffset = (1 - m_readBuffer) * NUM_BODIES; //Unsure about this

		m_buffer->SetConstantBufferData(&cbUpdate, sizeof(cbUpdate), frameIndex, &m_cbUpdateAddress[0]);
		m_buffer->BindConstantBufferForRootDescriptor(0, frameIndex, m_cbUpdateUploadHeap->GetAddressOf()); //Root index 0

		//Set NBody compute shader
		m_commandList->SetPipelineState(shader->GetShaders(Shaders::ID::NBodyCompute).pipelineState.Get());
		signature->SetComputeRootSignature();
		m_srvUavDescHeap->SetComputeRootDescriptorTable(1, m_srvUavDescHeap->GetGPUIncrementHandle(1)); //Root index 1 for UAV
		shader->SetComputeDispatch(NUM_BODIES / 256, 1, 1);

		//Copy the UAV data to the SRV
		m_buffer->SetResourceBarrier(m_uavBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		m_buffer->SetResourceBarrier(m_srvBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
		m_commandList->CopyResource(m_srvBuffer.Get(), m_uavBuffer.Get());
		m_buffer->SetResourceBarrier(m_srvBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		m_buffer->SetResourceBarrier(m_uavBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		m_readBuffer = 1 - m_readBuffer; //Unsure about this
	}

	void NBodySystemCS::SetPointSize(const float & size)
	{
		m_fPointSize = size;
	}

	float NBodySystemCS::GetPointSize() const
	{
		return m_fPointSize;
	}
}


