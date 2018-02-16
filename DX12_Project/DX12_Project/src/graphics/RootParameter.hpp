#pragma once
#include <d3d12.h>
#include <vector>

namespace dx
{
	class RootParameter
	{
	public:
		inline void AppendRootParameterCBV(const UINT & shaderRegister, D3D12_SHADER_VISIBILITY visibility)
		{
			D3D12_ROOT_PARAMETER1 param = {};
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			param.Descriptor = { 0, shaderRegister };
			param.ShaderVisibility = visibility;

			//Add the root parameter to the vector
			m_rootParameters.push_back(param);
		}

		inline void AppendRootParameterUAV(const UINT & shaderRegister, D3D12_SHADER_VISIBILITY visibility)
		{
			D3D12_ROOT_PARAMETER1 param = {};
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
			param.Descriptor = { 0, shaderRegister };
			param.ShaderVisibility = visibility;

			//Add the root parameter to the vector
			m_rootParameters.push_back(param);
		}

		inline void AppendRootParameterDescTable(D3D12_ROOT_DESCRIPTOR_TABLE1 table, D3D12_SHADER_VISIBILITY visibility)
		{
			D3D12_ROOT_PARAMETER1 param = {};
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			param.DescriptorTable = table;
			param.ShaderVisibility = visibility;

			//Add the root parameter to the vector
			m_rootParameters.push_back(param);
		}

	public:
		inline std::vector<D3D12_ROOT_PARAMETER1> & GetRootParameters()
		{
			return m_rootParameters;
		}

	private:
		std::vector<D3D12_ROOT_PARAMETER1> m_rootParameters;
	};
}