#pragma once
#include <d3dx12.h>
#include <wrl.h>
#include <string>
#include <map>
#include <vector>

namespace Shaders
{
	enum class ID
	{
		Triangle,
		Compute
	};
}

using namespace Microsoft::WRL;

namespace dx
{
	enum ShaderType
	{
		VS = 0,
		PS = 0x1,
		GS = 0x2,
		CS = 0x4
	};

	DEFINE_ENUM_FLAG_OPERATORS(ShaderType);

	class Shader
	{
	private:
		struct ShaderData
		{
			std::vector<ComPtr<ID3DBlob>> blobs;
			ComPtr<ID3D12PipelineState> pipelineState;
			ShaderType type;
		};

	public:
		Shader(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
		void LoadShadersFromFile(const Shaders::ID & id, const std::string & shaderPath, ShaderType type);
		void CreateInputLayoutAndPipelineState(const Shaders::ID & id, ID3D12RootSignature* signature);
		void CreatePipelineStateForComputeShader(const Shaders::ID & id, ID3D12RootSignature* signature);

	public:
		void SetTopology(D3D12_PRIMITIVE_TOPOLOGY topology);
		void SetComputeDispatch(const UINT & tgx, const UINT & tgy, const UINT & tgz);

	public:
		ShaderData GetShaders(const Shaders::ID & id) const;

	private:
		ID3D12Device * m_device;
		ID3D12GraphicsCommandList* m_commandList;
		std::map<Shaders::ID, ShaderData> m_shaders;
	};
}

