#pragma once
#include <DirectXMath.h>
#include <memory>
#include <graphics/Buffer.hpp>

using namespace DirectX;

namespace dx
{
	class Model
	{
	public:
		Model(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, Buffer* buffer);

	public:
		void BindBuffers(const UINT & frameIndex);
		void Draw();

	public:
		struct Vertex
		{
			XMFLOAT3 position;
			XMFLOAT2 uv;
		};

		struct CBInfo
		{
			XMFLOAT4 color;
		};

	private:
		ComPtr<ID3D12Resource> m_vertexBuffer;
		ComPtr<ID3D12Resource> m_indexBuffer;
		ComPtr<ID3D12Resource> m_vertexBufferUploadHeap;
		ComPtr<ID3D12Resource> m_indexBufferUploadHeap;
		ComPtr<ID3D12Resource> m_constantUploadHeap[2];
		UINT8* cbvGPUAddress[2];
		CBInfo m_cb;

	private:
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	private:
		Buffer * m_buffer;
		ID3D12Device * m_device;
		ID3D12GraphicsCommandList* m_commandList;
	};
}