#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <memory>
#include <wrl.h>

using namespace DirectX;
using namespace Microsoft::WRL;

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
		Vertex(float x, float y, float z, float u, float v) : position(x, y, z), uv(u, v) {}
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
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
};