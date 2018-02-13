#include <graphics/Shader.hpp>
#include <utils/Utility.hpp>
#include <assert.h>
#include <d3dcompiler.h>

namespace dx
{
	Shader::Shader(ID3D12Device * device, ID3D12GraphicsCommandList * commandList) : m_device(device), m_commandList(commandList)
	{
	}

	//Only vertex and pixel shader here for now...
	void Shader::LoadShaders(const Shaders::ID & id, const std::string & vertexPath, const std::string & pixelPath)
	{
		ShaderData data;
		CreateShaders(vertexPath, pixelPath, &data.byteCode[0], data.blobs->GetAddressOf());

		auto inserted = m_standardShaders.insert(std::make_pair(id, std::move(data)));
		assert(inserted.second);
	}

	//Compute shader
	void Shader::LoadComputeShader(const Shaders::ID & id, const std::string & computePath)
	{
		ComputeShaderData data;
		CreateComputeShader(computePath, data.byteCode, data.blob.GetAddressOf());

		auto inserted = m_computeShaders.insert(std::make_pair(id, std::move(data)));
		assert(inserted.second);
	}

	void Shader::CreateComputeShader(const std::string & computePath, D3D12_SHADER_BYTECODE & byteCode, ID3DBlob ** blob)
	{
		//Compile compute shader
		assert(!D3DCompileFromFile(ToWChar(computePath).c_str(), nullptr, nullptr, "main", "cs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &blob[0], nullptr));
		byteCode.BytecodeLength = blob[0]->GetBufferSize();
		byteCode.pShaderBytecode = blob[0]->GetBufferPointer();
	}

	void Shader::CreateShaders(const std::string & vertexPath, const std::string & pixelPath, D3D12_SHADER_BYTECODE * byteCode, ID3DBlob ** blobs)
	{
		//Compile vertex shader
		assert(!D3DCompileFromFile(ToWChar(vertexPath).c_str(), nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &blobs[0], nullptr));
		byteCode[0].BytecodeLength = blobs[0]->GetBufferSize();
		byteCode[0].pShaderBytecode = blobs[0]->GetBufferPointer();

		//Compile pixel shader
		assert(!D3DCompileFromFile(ToWChar(pixelPath).c_str(), nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &blobs[1], nullptr));
		byteCode[1].BytecodeLength = blobs[1]->GetBufferSize();
		byteCode[1].pShaderBytecode = blobs[1]->GetBufferPointer();
	}

	//Standard parameters for now
	void Shader::CreateInputLayoutAndPipelineState(const Shaders::ID & id, ID3D12RootSignature * signature)
	{
		auto found = m_standardShaders.find(id);

		//Just hardcoded input layout for now
		D3D12_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
		inputLayoutDesc.NumElements = ARRAYSIZE(inputElementDesc);
		inputLayoutDesc.pInputElementDescs = inputElementDesc;

		//Fill in the pipeline description
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = { 0 };
		pipelineStateDesc.InputLayout = inputLayoutDesc;
		pipelineStateDesc.pRootSignature = signature;
		pipelineStateDesc.VS = found->second.byteCode[0];
		pipelineStateDesc.PS = found->second.byteCode[1];
		pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		pipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		pipelineStateDesc.SampleDesc.Count = 1;
		pipelineStateDesc.SampleDesc.Quality = 0;
		pipelineStateDesc.SampleMask = 0xffffffff;
		pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		pipelineStateDesc.NumRenderTargets = 1;
		pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

		//Create a pipeline state object from the description
		assert(!m_device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(found->second.pipelineState.GetAddressOf())));

		//Release the blobs
		found->second.blobs[0].Reset();
		found->second.blobs[1].Reset();
	}

	void Shader::CreatePipelineStateForComputeShader(const Shaders::ID & id, ID3D12RootSignature * signature)
	{
		auto found = m_computeShaders.find(id);

		//Fill in compute pipeline description
		D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateDesc = { 0 };
		pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		pipelineStateDesc.CS = found->second.byteCode;
		pipelineStateDesc.pRootSignature = signature;

		//Create a pipeline state object from the description
		assert(!m_device->CreateComputePipelineState(&pipelineStateDesc, IID_PPV_ARGS(found->second.pipelineState.GetAddressOf())));

		//Release blob
		found->second.blob.Reset();
	}

	void Shader::SetTopology(D3D12_PRIMITIVE_TOPOLOGY topology)
	{
		m_commandList->IASetPrimitiveTopology(topology);
	}

	void Shader::SetComputeDispatch(const UINT & tgx, const UINT & tgy, const UINT & tgz)
	{
		m_commandList->Dispatch(tgx, tgy, tgz);
	}

	Shader::ShaderData Shader::GetShaders(const Shaders::ID & id) const
	{
		auto found = m_standardShaders.find(id);
		assert(found != m_standardShaders.end());
		return found->second;
	}
}
