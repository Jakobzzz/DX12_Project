#include <graphics/nbody/nBody.hpp>

//Constant buffer for rendering particles
struct CB_DRAW
{
	Matrix g_mWorldViewProjection;
};

//Constant buffer for the simulation update compute shader
struct CB_UPDATE
{
	float g_timestep;
    float g_softeningSquared;
	UINT g_numParticles;
};

FLOAT blendFactors[] = { 1.0f, 1.0f, 1.0f, 1.0f };

namespace dx
{
	NBody::NBody(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, Buffer* buffer, Camera* camera, Texture* texture) : m_device(device), 
								 m_commandList(commandList), m_buffer(buffer), m_camera(camera), m_texture(texture), m_clusterScale(1.54f), m_velocityScale(8.0f)
	{
		Initialize();
		InitializeBodies();
	}

	//Create resources
	void NBody::Initialize()
	{
		//Create constant buffers (two for normal pipeline and one for compute shader)
		m_buffer->CreateConstantBuffer(m_cbDrawUploadHeap->GetAddressOf(), &m_cbDrawAddress[0]);
		m_buffer->CreateConstantBuffer(m_cbUpdateUploadHeap->GetAddressOf(), &m_cbUpdateAddress[0]);

		//Descriptor heap
		m_srvUavDescHeap = std::make_unique<DescriptorHeap>(m_device, m_commandList, 1);
		m_srvUavDescHeap->CreateDescriptorHeap(3, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	//Render the bodies as particles using sprites
	void NBody::RenderBodies(Shader* shader, RootSignature* signature, const UINT & frameIndex)
	{
		//Set constant buffer data for normal pipeline
		CB_DRAW cbDraw;
		Matrix world = XMMatrixIdentity();
		Matrix WVP = world * m_camera->GetViewProjectionMatrix();
		cbDraw.g_mWorldViewProjection = XMMatrixTranspose(WVP);

		m_buffer->SetConstantBufferData(&cbDraw, sizeof(cbDraw), frameIndex, &m_cbDrawAddress[0]);

		//Set the normal NBody shader and root signature
		m_commandList->OMSetBlendFactor(blendFactors);
		m_commandList->SetPipelineState(shader->GetShaders(Shaders::ID::NBody).pipelineState.Get());
		signature->SetRootSignature();
		m_buffer->BindConstantBufferForRootDescriptor(0, frameIndex, m_cbDrawUploadHeap->GetAddressOf()); //Root index 0
		m_srvUavDescHeap->SetRootDescriptorTable(1, m_srvUavDescHeap->GetGPUIncrementHandle(0)); //Root index 1 for SRV table
		shader->SetTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

		//Draw particles
		m_commandList->DrawInstanced(NUM_BODIES, 1, 0, 0);
	}

	//Update the positions and velocities of all bodies in the system
	void NBody::UpdateBodies(Shader* shader, RootSignature* signature, const UINT & frameIndex)
	{
		//Update the data for the compute constant buffer
		CB_UPDATE cbUpdate;
		cbUpdate.g_timestep = 0.016f;
		cbUpdate.g_softeningSquared = 0.01f;
		cbUpdate.g_numParticles = NUM_BODIES;
		m_buffer->SetConstantBufferData(&cbUpdate, sizeof(cbUpdate), frameIndex, &m_cbUpdateAddress[0]);

		//Set NBody compute shader
		m_commandList->SetPipelineState(shader->GetShaders(Shaders::ID::NBodyCompute).pipelineState.Get());
		signature->SetComputeRootSignature();
		m_buffer->BindConstantBufferComputeForRootDescriptor(0, frameIndex, m_cbUpdateUploadHeap->GetAddressOf()); //Root index 0
		m_srvUavDescHeap->SetComputeRootDescriptorTable(1, m_srvUavDescHeap->GetGPUIncrementHandle(2)); //Root index 1 for UAV table
		shader->SetComputeDispatch(static_cast<int>(ceil(NUM_BODIES / 256)), 1, 1);

		//Copy the UAV data to the SRV
		m_buffer->SetResourceBarrier(m_uavBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		m_buffer->SetResourceBarrier(m_srvBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
		m_commandList->CopyResource(m_srvBuffer.Get(), m_uavBuffer.Get());
		m_buffer->SetResourceBarrier(m_srvBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		m_buffer->SetResourceBarrier(m_uavBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}

	void NBody::InitializeBodies()
	{
		BodyData* bodyData = new BodyData[NUM_BODIES];
		float vscale = m_clusterScale * m_velocityScale;
		float inner = 2.5f * m_clusterScale;
		float outer = 4.0f * m_clusterScale;

		unsigned int i = 0;
		while (i < NUM_BODIES)
		{
			Vector4 point;
			point.x = rand() / (float)RAND_MAX * 2 - 1;
			point.y = rand() / (float)RAND_MAX * 2 - 1;
			point.z = rand() / (float)RAND_MAX * 2 - 1;
			point.w = 1.f;
			point.Normalize();
			
			//Init positions
			point.x *= (inner + (outer - inner) * rand() / (float)RAND_MAX);
			point.y *= (inner + (outer - inner) * rand() / (float)RAND_MAX);
			point.z *= (inner + (outer - inner) * rand() / (float)RAND_MAX);
			bodyData[i].position = Vector4(point.x, point.y, point.z, 1.0f);

			//Init velocities
			Vector4 axis = Vector4(0.f, 0.f, 1.f, 1.f);
			axis.Normalize();

			if ((1 - (point.Dot(axis))) < 1e-6)
			{
				axis.x = point.y;
				axis.y = point.x;
				axis.Normalize();
			}

			Vector4 velocity = Vector4(bodyData[i].position);
			auto res = velocity.Cross(velocity, axis);
			bodyData[i].velocity = Vector4(res.x * vscale, res.y * vscale, res.z * vscale, 1.f);
			i++;
		}

		//Create SRV buffer for normal pipeline
		m_buffer->CreateSRVForRootTable(bodyData, sizeof(BodyData) * NUM_BODIES, sizeof(BodyData), NUM_BODIES, m_srvBuffer.GetAddressOf(),
			m_srvBufferUploadHeap.GetAddressOf(), m_srvUavDescHeap->GetCPUIncrementHandle(0), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		//Create SRV from texture
		m_texture->CreateSRVFromTexture(Textures::ID::Particle, m_srvUavDescHeap->GetCPUIncrementHandle(1));

		//Create UAV buffer for compute shader
		m_buffer->CreateUAVForRootTable(bodyData, sizeof(BodyData) * NUM_BODIES, sizeof(BodyData), NUM_BODIES, m_uavBuffer.GetAddressOf(),
			m_uavBufferUploadHeap.GetAddressOf(), m_srvUavDescHeap->GetCPUIncrementHandle(2));

		//Release memory
		delete[] bodyData;
		bodyData = nullptr;
	}
}


