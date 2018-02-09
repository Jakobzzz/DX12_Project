#include <graphics/DescriptorHeap.hpp>
#include <assert.h>
#include <memory>

namespace dx
{
	DescriptorHeap::DescriptorHeap(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) : m_device(device), m_commandList(commandList)
	{
	}

	void DescriptorHeap::CreateDescriptorHeap(const UINT & numDesc, D3D12_DESCRIPTOR_HEAP_FLAGS heapFlags, D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = numDesc;
		heapDesc.Flags = heapFlags;
		heapDesc.Type = type;
		m_handleIncrementSize = m_device->GetDescriptorHandleIncrementSize(heapDesc.Type);

		//Create the descriptor heap from the description
		assert(!m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_descHeap.GetAddressOf())));
	}

	void DescriptorHeap::SetRootDescriptorTable(const UINT & rootIndex)
	{
		ID3D12DescriptorHeap* descriptorHeaps[] = { m_descHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		m_commandList->SetGraphicsRootDescriptorTable(rootIndex, m_descHeap->GetGPUDescriptorHandleForHeapStart()); //Note: We'll see if this needs a change in the future!
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUIncrementHandle(const INT & resourceIndex)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = { m_descHeap->GetCPUDescriptorHandleForHeapStart(), resourceIndex, m_handleIncrementSize };
		return handle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUIncrementHandle(const INT & resourceIndex)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE handle = { m_descHeap->GetGPUDescriptorHandleForHeapStart(), resourceIndex, m_handleIncrementSize };
		return handle;
	}

	ID3D12DescriptorHeap * DescriptorHeap::GetDescriptorHeap() const
	{
		return m_descHeap.Get();
	}
}