#pragma once
#include <d3dx12.h>
#include <wrl.h>
#include <vector>

using namespace Microsoft::WRL;

namespace dx
{
	class DescriptorHeap
	{
	public:
		DescriptorHeap(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const UINT & numHeaps);
		void CreateDescriptorHeap(const UINT & numDesc, D3D12_DESCRIPTOR_HEAP_FLAGS heapFlags, D3D12_DESCRIPTOR_HEAP_TYPE type);

	public:
		void SetRootDescriptorTable(const UINT & rootIndex, const UINT & frameIndex = 0);

	public:
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUIncrementHandle(const INT & resourceIndex);
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> GetCPUIncrementHandleForMultipleHeaps(const INT & resourceIndex);

	private:
		std::vector<ComPtr<ID3D12DescriptorHeap>> m_descHeaps;
		UINT m_handleIncrementSize;

	private:
		ID3D12Device* m_device;
		ID3D12GraphicsCommandList* m_commandList;
	};
}