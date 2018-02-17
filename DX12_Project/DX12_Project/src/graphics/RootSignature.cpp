#include <graphics/RootSignature.hpp>
#include <assert.h>
#include <utils/Utility.hpp>

namespace dx
{
	RootSignature::RootSignature(ID3D12Device * device, ID3D12GraphicsCommandList* commandList, ID3D12GraphicsCommandList* computeCommandList) : 
		m_device(device), m_commandList(commandList), m_computeCommandList(computeCommandList)
	{
	}

	void RootSignature::CreateRootSignature(const UINT & size, const UINT & staticSamplers, const D3D12_ROOT_PARAMETER1 * params, 
											const D3D12_STATIC_SAMPLER_DESC * samplers, D3D12_ROOT_SIGNATURE_FLAGS flags)
	{
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootDesc = {};
		rootDesc.Init_1_1(size, params, staticSamplers, samplers, flags);

		//Create signature from the description
		ID3DBlob* errorBuff; ID3DBlob* signature;
		assert(!D3DX12SerializeVersionedRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &errorBuff));
		assert(!m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.GetAddressOf())));

		//Release the blobs since we no longer need them
		SAFE_RELEASE(&errorBuff);
		SAFE_RELEASE(&signature);
	}

	void RootSignature::SetRootSignature()
	{
		m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	}

	void RootSignature::SetComputeRootSignature()
	{
		m_computeCommandList->SetComputeRootSignature(m_rootSignature.Get());
	}

	ID3D12RootSignature * RootSignature::GetRootSignature() const
	{
		return m_rootSignature.Get();
	}
}