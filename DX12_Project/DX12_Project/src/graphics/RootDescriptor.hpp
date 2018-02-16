#pragma once
#include <d3dx12.h>
#include <vector>

namespace dx
{
	class RootDescriptor
	{
	public:
		//Call this after you have appended all the parameters to the descriptor range
		inline void CreateRootDescTable()
		{
			m_rootDescTable = { (UINT)m_descRange.size(), &m_descRange[0] };
		}

		inline void AppendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type, const UINT & numDesc, const UINT & baseShaderRegister, D3D12_DESCRIPTOR_RANGE_FLAGS flags)
		{
			m_descRange.push_back({ type, numDesc, baseShaderRegister, 0, flags });
		}

	public:
		inline D3D12_ROOT_DESCRIPTOR_TABLE1 GetRootDescTable() const
		{
			return m_rootDescTable;
		}

	private:
		std::vector<CD3DX12_DESCRIPTOR_RANGE1> m_descRange;
		CD3DX12_ROOT_DESCRIPTOR_TABLE1 m_rootDescTable;
	};
}
