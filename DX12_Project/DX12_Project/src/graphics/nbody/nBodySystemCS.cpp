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

	//-----------------------------------------------------------------------------
	// Render the bodies as particles using sprites
	//-----------------------------------------------------------------------------
	//void NBodySystemCS::RenderBodies(const Matrix *p_mWorld)
	//{
	//	unsigned int particleCount = 0;

	//	D3DXMATRIX  mWorldViewProjection;
	//	D3DXMatrixMultiply(&mWorldView, p_mWorld, p_mView);
	//	D3DXMatrixMultiply(&mWorldViewProjection, &mWorldView, p_mProj);

	//	// Set the input layout
	//	m_pd3dImmediateContext->IASetInputLayout(m_pIndLayout);
	//	m_pd3dImmediateContext->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, 0);

	//	D3DXMatrixTranspose(&mWorldViewProjection, &mWorldViewProjection);

	//	// constant buffers
	//	D3D11_MAPPED_SUBRESOURCE mappedResource;
	//	V(m_pd3dImmediateContext->Map(m_pcbDraw, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	//	CB_DRAW* pcbDraw = (CB_DRAW*)mappedResource.pData;
	//	pcbDraw->g_mWorldViewProjection = mWorldViewProjection;
	//	pcbDraw->g_fPointSize = m_fPointSize;
	//	pcbDraw->g_readOffset = m_readBuffer * m_numBodies;
	//	m_pd3dImmediateContext->Unmap(m_pcbDraw, 0);

	//	particleCount = m_numBodies;
	//	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	//	m_pd3dImmediateContext->PSSetShaderResources(0, 1, &m_pParticleTexSRV);
	//	m_pd3dImmediateContext->PSSetSamplers(0, 1, &m_pParticleSamplerState);

	//	m_pd3dImmediateContext->VSSetShaderResources(0, 1, &m_pStructuredBufferSRV);

	//	m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, &m_pcbDraw);
	//	m_pd3dImmediateContext->VSSetConstantBuffers(1, 1, &m_pcbImmutable);
	//	m_pd3dImmediateContext->GSSetConstantBuffers(0, 1, &m_pcbImmutable);
	//	m_pd3dImmediateContext->PSSetConstantBuffers(0, 1, &m_pcbDraw);

	//	// Set shaders
	//	m_pd3dImmediateContext->GSSetShader(m_pGSDisplayParticle, NULL, 0);
	//	m_pd3dImmediateContext->VSSetShader(m_pVSDisplayParticleStructBuffer, NULL, 0);
	//	m_pd3dImmediateContext->PSSetShader(m_pPSDisplayParticleTex, NULL, 0);

	//	m_pd3dImmediateContext->RSSetState(m_pRasterizerState);

	//	// Set Blending State
	//	m_pd3dImmediateContext->OMSetBlendState(m_pBlendingEnabled, blendFactors, 0xFFFFFFFF);

	//	// Set Depth State
	//	m_pd3dImmediateContext->OMSetDepthStencilState(m_pDepthDisabled, 0);

	//	m_pd3dImmediateContext->Draw(particleCount, 0);

	//	// Unbound all PS shader resources
	//	ID3D11ShaderResourceView *const pSRV[4] = { NULL, NULL, NULL, NULL };
	//	m_pd3dImmediateContext->PSSetShaderResources(0, 4, pSRV);
	//	m_pd3dImmediateContext->VSSetShaderResources(0, 4, pSRV);
	//	ID3D11Buffer *const pCB[4] = { NULL, NULL, NULL, NULL };
	//	m_pd3dImmediateContext->PSSetConstantBuffers(0, 4, pCB);
	//	m_pd3dImmediateContext->GSSetConstantBuffers(0, 4, pCB);

	//	return hr;
	//}

	// Update the positions and velocities of all bodies in the system
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

		//Set NBody compute shader
		m_commandList->SetPipelineState(shader->GetShaders(Shaders::ID::NBodyCompute).pipelineState.Get());
		signature->SetComputeRootSignature();
		m_srvUavDescHeap->SetComputeRootDescriptorTable(0, m_srvUavDescHeap->GetGPUIncrementHandle(1));
		shader->SetComputeDispatch(1, 1, 1);

		//Copy the UAV data to the SRV
		/*m_buffer->SetResourceBarrier(m_uavBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		m_buffer->SetResourceBarrier(m_srvBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

		m_commandList->CopyResource(m_srvBuffer.Get(), m_uavBuffer.Get());

		m_buffer->SetResourceBarrier(m_srvBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		m_buffer->SetResourceBarrier(m_uavBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);*/

		m_readBuffer = 1 - m_readBuffer; //Unsure about this
	}

	//Reset to initial state
	void NBodySystemCS::ResetBodies(BodyData* configData)
	{
		// for compute shader on CS_4_0, we can only have a single UAV per shader, so we have to store particle
		// position and velocity in the same array: all positions followed by all velocities (this will change)

		//Create SRV buffer for normal pipeline
		m_buffer->CreateSRVForRootTable(configData, sizeof(configData), sizeof(BodyData), NUM_BODIES, m_srvBuffer.GetAddressOf(), 
										m_srvBufferUploadHeap.GetAddressOf(), m_srvUavDescHeap->GetCPUIncrementHandle(0));


		//Create UAV buffer for compute shader
		m_buffer->CreateUAVForRootTable(configData, sizeof(configData), sizeof(BodyData), NUM_BODIES, m_uavBuffer.GetAddressOf(), 
										m_uavBufferUploadHeap.GetAddressOf(), m_srvUavDescHeap->GetCPUIncrementHandle(1));
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
}


