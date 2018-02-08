#pragma once
#include <d3d12.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace dx
{
	class Buffer
	{
	public:
		Buffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

	public:
		void CreateVertexBuffer(const void* data, const unsigned int & size, const unsigned int & stride, ID3D12Resource** buffer, 
								ID3D12Resource** uploadHeap, D3D12_VERTEX_BUFFER_VIEW & view);
		void CreateIndexBuffer(const void* data, const unsigned int & size, ID3D12Resource** buffer, ID3D12Resource** uploadHeap, D3D12_INDEX_BUFFER_VIEW & view);
		void Bind(const unsigned int & location, D3D12_VERTEX_BUFFER_VIEW & view);
		void Bind(D3D12_INDEX_BUFFER_VIEW & view);

	private:
		void CreateBuffer(const void* data, const unsigned int & size, ID3D12Resource** buffer, ID3D12Resource** uploadHeap);

	private:
		ID3D12Device* m_device;
		ID3D12GraphicsCommandList* m_commandList;
	};
}