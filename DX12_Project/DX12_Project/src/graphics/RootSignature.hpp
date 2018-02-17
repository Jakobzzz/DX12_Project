#pragma once
#include <d3dx12.h>
#include <wrl.h>

using namespace Microsoft::WRL;

namespace dx
{
	class RootSignature
	{
	public:
		RootSignature(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12GraphicsCommandList* computeCommandList);
		void CreateRootSignature(const UINT & size, const UINT & staticSamplers, const D3D12_ROOT_PARAMETER1* params,
			const D3D12_STATIC_SAMPLER_DESC* samplers, D3D12_ROOT_SIGNATURE_FLAGS flags);

	public:
		void SetRootSignature();
		void SetComputeRootSignature();

	public:
		ID3D12RootSignature * GetRootSignature() const;

	private:
		ComPtr<ID3D12RootSignature> m_rootSignature;
		ID3D12Device* m_device;
		ID3D12GraphicsCommandList* m_commandList;
		ID3D12GraphicsCommandList* m_computeCommandList;
	};
}