#pragma once
#include <d3d12.h>
#include <wrl.h>

using namespace Microsoft::WRL;

//Utility class for all kinds of buffers
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
		void CreateConstantBufferForRoot(const void* data, const unsigned int & size, ID3D12Resource** buffer, UINT8** bufferAddress);

	public:
		void SetConstantBufferData(const void* data, const unsigned int & size, const unsigned int & frameIndex, UINT8** bufferAddress);
		void BindVertexBuffer(const unsigned int & location, D3D12_VERTEX_BUFFER_VIEW & view);
		void BindIndexBuffer(D3D12_INDEX_BUFFER_VIEW & view);
		void BindConstantBufferForRoot(const unsigned int & rootIndex, const unsigned int & frameIndex, ID3D12Resource** buffer);

	private:
		void CreateBuffer(const void* data, const unsigned int & size, ID3D12Resource** buffer, ID3D12Resource** uploadHeap);

	private:
		ID3D12Device* m_device;
		ID3D12GraphicsCommandList* m_commandList;
	};
}