#pragma once
#include <d3dx12.h>
#include <wrl.h>
#include <string>
#include <map>

namespace Shaders
{
	enum class ID
	{
		Triangle
	};
}

using namespace Microsoft::WRL;

namespace dx
{
	class Shader
	{
	private:
		struct ShaderData
		{
			D3D12_SHADER_BYTECODE byteCode[2];
			ComPtr<ID3DBlob> blobs[2];
			ComPtr<ID3D12PipelineState> pipelineState;
		};

	public:
		Shader(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
		void LoadShaders(const Shaders::ID & id, const std::string & vertexPath, const std::string & pixelPath);
		void CreateInputLayoutAndPipelineState(const Shaders::ID & id, ID3D12RootSignature* signature);

	public:
		void SetTopology(D3D12_PRIMITIVE_TOPOLOGY topology);

	public:
		ShaderData GetShaders(const Shaders::ID & id) const;

	private:
		void CreateShaders(const std::string & vertexPath, const std::string & pixelPath, D3D12_SHADER_BYTECODE* byteCode, ID3DBlob** blobs);

	private:
		ID3D12Device * m_device;
		ID3D12GraphicsCommandList* m_commandList;
		std::map<Shaders::ID, ShaderData> m_standardShaders;
	};
}

