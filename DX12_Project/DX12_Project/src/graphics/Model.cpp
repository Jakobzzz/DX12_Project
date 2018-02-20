#include <graphics/Model.hpp>
#include <utils/Utility.hpp>

namespace dx
{
	//One triangle
	Model::Vertex vertices[] =
	{
		XMFLOAT3({ 0.0f, 0.05f, 0.0f })
	};

	Model::Model(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, Buffer* buffer, Camera* camera) : m_device(device), m_commandList(commandList), 
																												 m_buffer(buffer), m_camera(camera)
	{
		//Create vertex and constant buffer
		m_buffer->CreateVertexBuffer(vertices, sizeof(vertices), sizeof(Vertex), m_vertexBuffer.GetAddressOf(), m_vertexBufferUploadHeap.GetAddressOf(),
									 m_vertexBufferView);
		m_buffer->CreateConstantBuffer(m_constantUploadHeap->GetAddressOf(), &m_cbvGPUAddress[0]);
	}

	void Model::BindBuffers(const UINT & rootIndex, const UINT & frameIndex)
	{
		m_world = XMMatrixIdentity();
		m_WVP = m_world * m_camera->GetViewProjectionMatrix();
		m_cb.WVP = XMMatrixTranspose(m_WVP);

		m_buffer->SetConstantBufferData(&m_cb, sizeof(m_cb), frameIndex, &m_cbvGPUAddress[0]);
		m_buffer->BindVertexBuffer(0, m_vertexBufferView);
		m_buffer->BindConstantBufferForRootDescriptor(rootIndex, frameIndex, m_constantUploadHeap->GetAddressOf());
	}

	void Model::Draw()
	{
		m_commandList->DrawInstanced(ARRAYSIZE(vertices), 1, 0, 0); //Just a triangle for now
	}
}