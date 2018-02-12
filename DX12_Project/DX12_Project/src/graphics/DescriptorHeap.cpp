#include <graphics/DescriptorHeap.hpp>
#include <assert.h>
#include <memory>

namespace dx
{
	DescriptorHeap::DescriptorHeap(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const UINT & numHeaps) : m_device(device), 
								   m_commandList(commandList), m_descHeaps(numHeaps)
	{
	}

	void DescriptorHeap::CreateDescriptorHeap(const UINT & numDesc, D3D12_DESCRIPTOR_HEAP_FLAGS heapFlags, D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		for (UINT i = 0; i < m_descHeaps.size(); ++i)
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
			heapDesc.NumDescriptors = numDesc;
			heapDesc.Flags = heapFlags;
			heapDesc.Type = type;

			//Create the descriptor heap from the description
			assert(!m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_descHeaps[i].GetAddressOf())));
		}

		//Store increment size of the desc type
		m_handleIncrementSize = m_device->GetDescriptorHandleIncrementSize(type);
	}

	void DescriptorHeap::SetRootDescriptorTable(const UINT & rootIndex, const UINT & frameIndex)
	{
		if (m_descHeaps.size() > 1)
		{
			ID3D12DescriptorHeap* descriptorHeaps[] = { m_descHeaps[frameIndex].Get() };
			m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
			m_commandList->SetGraphicsRootDescriptorTable(rootIndex, m_descHeaps[frameIndex]->GetGPUDescriptorHandleForHeapStart());
		}
		else
		{
			ID3D12DescriptorHeap* descriptorHeaps[] = { m_descHeaps[0].Get() };
			m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
			m_commandList->SetGraphicsRootDescriptorTable(rootIndex, m_descHeaps[0]->GetGPUDescriptorHandleForHeapStart());
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUIncrementHandle(const INT & resourceIndex)
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_descHeaps[0]->GetCPUDescriptorHandleForHeapStart(), resourceIndex, m_handleIncrementSize);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUIncrementHandle(const INT & resourceIndex)
	{
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descHeaps[0]->GetGPUDescriptorHandleForHeapStart(), resourceIndex, m_handleIncrementSize);
	}
}