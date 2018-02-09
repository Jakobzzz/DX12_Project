#pragma once
#include <d3dx12.h>
#include <vector>

namespace dx
{
	class RootDescriptor
	{
	public:
		void CreateRootDescTable();
		void AppendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type, const unsigned int & numDesc, const unsigned int & baseShaderRegister);

	public:
		CD3DX12_ROOT_DESCRIPTOR_TABLE GetRootDescTable() const;

	private:
		std::vector<CD3DX12_DESCRIPTOR_RANGE> m_descRange;
		CD3DX12_ROOT_DESCRIPTOR_TABLE m_rootDescTable;
	};

	inline void RootDescriptor::CreateRootDescTable()
	{
		m_rootDescTable = { m_descRange.size(), &m_descRange[0] };
	}

	inline void RootDescriptor::AppendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type, const unsigned int & numDesc, const unsigned int & baseShaderRegister)
	{
		m_descRange.push_back({ type, numDesc, baseShaderRegister });
	}

	inline CD3DX12_ROOT_DESCRIPTOR_TABLE RootDescriptor::GetRootDescTable() const
	{
		return m_rootDescTable;
	}
}
