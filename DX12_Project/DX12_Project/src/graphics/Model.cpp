#include <graphics/Model.hpp>
#include <utils/Utility.hpp>
#include <d3dx12.h>

Model::Vertex vertices[] =
{
	XMFLOAT3({ 0.0f, 0.05f, 0.0f }), XMFLOAT2({ 0.5f , -0.99f }),
	XMFLOAT3({ 0.05f, -0.05f, 0.0f }), XMFLOAT2({ 1.49f, 1.1f }),
	XMFLOAT3({ -0.05f,  -0.05f, 0.0f }), XMFLOAT2({ -0.51f, 1.1f })
};

DWORD indices[] =
{
	0, 1, 2, //First triangle
	0, 2, 3 //Second triangle
};


Model::Model(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) : m_device(device), m_commandList(commandList)
{
	m_buffer = std::make_unique<dx::Buffer>(m_device, m_commandList);
}

void Model::CreateVertexBuffer()
{
	m_buffer->CreateVertexBuffer(vertices, sizeof(vertices), sizeof(Vertex), m_vertexBuffer.GetAddressOf(), m_vertexBufferUploadHeap.GetAddressOf(), 
								 m_vertexBufferView);
}

void Model::Render(ID3D12GraphicsCommandList* commandList)
{
	//Prepare for drawing
	m_buffer->Bind(0, m_vertexBufferView);
	m_buffer->Bind(m_indexBufferView);
	commandList->DrawInstanced(ARRAYSIZE(vertices), 1, 0, 0); //Just a triangle for now
}

void Model::CreateIndexBuffer()
{
	m_buffer->CreateIndexBuffer(indices, sizeof(indices), m_indexBuffer.GetAddressOf(), m_indexBufferUploadHeap.GetAddressOf(), m_indexBufferView);
}
