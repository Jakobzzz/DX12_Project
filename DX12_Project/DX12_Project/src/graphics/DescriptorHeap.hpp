#pragma once
#include <d3dx12.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace dx
{
	class DescriptorHeap
	{
	public:
		DescriptorHeap(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
		void CreateDescriptorHeap(const UINT & numDesc, D3D12_DESCRIPTOR_HEAP_FLAGS heapFlags, D3D12_DESCRIPTOR_HEAP_TYPE type);

	public:
		void SetRootDescriptorTable(const UINT & rootIndex);

	public:
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUIncrementHandle(const INT & resourceIndex);
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUIncrementHandle(const INT & resourceIndex);
		ID3D12DescriptorHeap * GetDescriptorHeap() const;

	private:
		ComPtr<ID3D12DescriptorHeap> m_descHeap;
		UINT m_handleIncrementSize;

	private:
		ID3D12Device* m_device;
		ID3D12GraphicsCommandList* m_commandList;
	};
}