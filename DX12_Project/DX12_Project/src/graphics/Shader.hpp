#pragma once
#include <d3dx12.h>
#include <wrl.h>
#include <string>
#include <map>

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
	class Shader
	{
	private:
		//Need to change this up, but just dirty version for now
		//probably just need a vector of blobs for fix...
		struct ShaderData
		{
			ComPtr<ID3DBlob> blobs[2];
			ComPtr<ID3D12PipelineState> pipelineState;
		};

		struct ComputeShaderData
		{
			ComPtr<ID3DBlob> blob;
			ComPtr<ID3D12PipelineState> pipelineState;
		};

	public:
		Shader(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
		void LoadShaders(const Shaders::ID & id, const std::string & vertexPath, const std::string & pixelPath);
		void LoadComputeShader(const Shaders::ID & id, const std::string & computePath);
		void CreateInputLayoutAndPipelineState(const Shaders::ID & id, ID3D12RootSignature* signature);
		void CreatePipelineStateForComputeShader(const Shaders::ID & id, ID3D12RootSignature* signature);

	public:
		void SetTopology(D3D12_PRIMITIVE_TOPOLOGY topology);
		void SetComputeDispatch(const UINT & tgx, const UINT & tgy, const UINT & tgz);

	public:
		ShaderData GetShaders(const Shaders::ID & id) const;
		ComputeShaderData GetComputeShader(const Shaders::ID & id) const;

	private:
		void CreateComputeShader(const std::string & computePath, ID3DBlob** blob);
		void CreateShaders(const std::string & vertexPath, const std::string & pixelPath, ID3DBlob** blobs);

	private:
		ID3D12Device * m_device;
		ID3D12GraphicsCommandList* m_commandList;
		std::map<Shaders::ID, ShaderData> m_standardShaders;
		std::map<Shaders::ID, ComputeShaderData> m_computeShaders;
	};
}

