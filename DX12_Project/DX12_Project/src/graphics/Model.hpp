#pragma once
#include <DirectXMath.h>
#include <memory>
#include <graphics/Buffer.hpp>

using namespace DirectX;

class Model
{
public:
	Model(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

public:
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void Render(ID3D12GraphicsCommandList*);

public:
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT2 uv;
	};

private:
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_vertexBufferUploadHeap;
	ComPtr<ID3D12Resource> m_indexBufferUploadHeap;
	ID3D12Device* m_device;
	ID3D12GraphicsCommandList* m_commandList;

private:
	std::unique_ptr<dx::Buffer> m_buffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
};