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
		//Create buffers
		m_buffer->CreateVertexBuffer(vertices, sizeof(vertices), sizeof(Vertex), m_vertexBuffer.GetAddressOf(), m_vertexBufferUploadHeap.GetAddressOf(),
			m_vertexBufferView);
		m_buffer->CreateIndexBuffer(indices, sizeof(indices), m_indexBuffer.GetAddressOf(), m_indexBufferUploadHeap.GetAddressOf(), m_indexBufferView);
	}

	void Model::Draw()
	{
		//Prepare for drawing
		m_buffer->BindVertexBuffer(0, m_vertexBufferView);
		m_buffer->BindIndexBuffer(m_indexBufferView);
		m_commandList->DrawInstanced(ARRAYSIZE(vertices), 1, 0, 0); //Just a triangle for now
	}
}