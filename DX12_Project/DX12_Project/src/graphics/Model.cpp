#include <graphics/Model.hpp>
#include <utils/Utility.hpp>

namespace dx
{
	//One triangle
	Model::Vertex vertices[] =
	{
		XMFLOAT3({ 0.0f, 0.05f, 0.0f }), XMFLOAT2({ 0.5f , -0.99f }),
		XMFLOAT3({ 0.05f, -0.05f, 0.0f }), XMFLOAT2({ 1.49f, 1.1f }),
		XMFLOAT3({ -0.05f,  -0.05f, 0.0f }), XMFLOAT2({ -0.51f, 1.1f })
	};

	//Two triangles
	DWORD indices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	Model::Model(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, Buffer* buffer) : m_device(device), m_commandList(commandList), m_buffer(buffer)
	{
		//Create vertex, index and constant buffer
		m_buffer->CreateVertexBuffer(vertices, sizeof(vertices), sizeof(Vertex), m_vertexBuffer.GetAddressOf(), m_vertexBufferUploadHeap.GetAddressOf(),
			m_vertexBufferView);
		m_buffer->CreateIndexBuffer(indices, sizeof(indices), m_indexBuffer.GetAddressOf(), m_indexBufferUploadHeap.GetAddressOf(), m_indexBufferView);
	}

	void Model::CreateConstantBuffers()
	{
		m_view = XMMatrixLookAtLH(Vector3(0.0f, 0.0f, -0.5f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.f, 0.0f));
		m_projection = XMMatrixPerspectiveFovLH(0.4f * 3.14f, static_cast<float>(SCREEN_WIDTH / SCREEN_HEIGHT), 0.1f, 1000.0f);
		m_buffer->CreateConstantBufferForRootDescriptor(m_constantUploadHeap->GetAddressOf(), &m_cbvGPUAddress[0]);
	}

	void Model::BindBuffers(const UINT & rootIndex, const UINT & frameIndex)
	{
		m_world = XMMatrixIdentity();
		m_WVP = m_world * m_view * m_projection;
		m_cb.WVP = XMMatrixTranspose(m_WVP);

		m_buffer->SetConstantBufferData(&m_cb, sizeof(m_cb), frameIndex, &m_cbvGPUAddress[0]);
		m_buffer->BindVertexBuffer(0, m_vertexBufferView);
		m_buffer->BindIndexBuffer(m_indexBufferView);
		m_buffer->BindConstantBufferForRootDescriptor(rootIndex, frameIndex, m_constantUploadHeap->GetAddressOf());
	}

	void Model::Draw()
	{
		m_commandList->DrawInstanced(ARRAYSIZE(vertices), 1, 0, 0); //Just a triangle for now
	}
}