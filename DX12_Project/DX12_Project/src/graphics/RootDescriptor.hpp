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
			m_rootDescTable = { m_descRange.size(), &m_descRange[0] };
		}

		inline void AppendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type, const unsigned int & numDesc, const unsigned int & baseShaderRegister)
		{
			m_descRange.push_back({ type, numDesc, baseShaderRegister });
		}

	public:
		inline CD3DX12_ROOT_DESCRIPTOR_TABLE GetRootDescTable() const
		{
			return m_rootDescTable;
		}

	private:
		std::vector<CD3DX12_DESCRIPTOR_RANGE> m_descRange;
		CD3DX12_ROOT_DESCRIPTOR_TABLE m_rootDescTable;
	};
}
