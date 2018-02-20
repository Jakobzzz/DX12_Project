#include <graphics/Shader.hpp>
#include <utils/Utility.hpp>
#include <assert.h>
#include <d3dcompiler.h>

namespace dx
{
	Shader::Shader(ID3D12Device * device, ID3D12GraphicsCommandList * commandList) : m_device(device), m_commandList(commandList)
	{
	}

	void Shader::LoadShadersFromFile(const Shaders::ID & id, const std::string & shaderPath, ShaderType type)
	{
		ShaderData data;
		data.type = type;

		//Compile depending on shader type
		if (data.type == (VS | PS))
		{
			data.blobs.resize(2);
			assert(!D3DCompileFromFile(ToWChar(shaderPath).c_str(), nullptr, nullptr, "VS_MAIN", "vs_5_1", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, data.blobs[0].GetAddressOf(), nullptr));
			assert(!D3DCompileFromFile(ToWChar(shaderPath).c_str(), nullptr, nullptr, "PS_MAIN", "ps_5_1", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, data.blobs[1].GetAddressOf(), nullptr));
		}
		else if (data.type == (VS | GS | PS))
		{
			data.blobs.resize(3);
			assert(!D3DCompileFromFile(ToWChar(shaderPath).c_str(), nullptr, nullptr, "VS_MAIN", "vs_5_1", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, data.blobs[0].GetAddressOf(), nullptr));
			assert(!D3DCompileFromFile(ToWChar(shaderPath).c_str(), nullptr, nullptr, "PS_MAIN", "ps_5_1", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, data.blobs[1].GetAddressOf(), nullptr));
			assert(!D3DCompileFromFile(ToWChar(shaderPath).c_str(), nullptr, nullptr, "GS_MAIN", "gs_5_1", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, data.blobs[2].GetAddressOf(), nullptr));
		}
		else if (data.type == CS)
		{
			data.blobs.resize(1);
			assert(!D3DCompileFromFile(ToWChar(shaderPath).c_str(), nullptr, nullptr, "CS_MAIN", "cs_5_1", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, data.blobs[0].GetAddressOf(), nullptr));
		}

		auto inserted = m_shaders.insert(std::make_pair(id, std::move(data)));
		assert(inserted.second);
	}

	//Standard parameters for now
	void Shader::CreateInputLayoutAndPipelineState(const Shaders::ID & id, ID3D12RootSignature * signature, D3D12_RASTERIZER_DESC rasterDesc,  D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType)
	{
		auto found = m_shaders.find(id);

		//Input layouts
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDesc;

		if (id == Shaders::ID::Triangle)
		{
			inputElementDesc =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};
		}
		else if (id == Shaders::ID::NBody)
		{
			inputElementDesc =
			{
				{ "POSINDEX", 0, DXGI_FORMAT_R32_UINT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
		}

		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
		inputLayoutDesc.NumElements = (UINT)inputElementDesc.size();
		inputLayoutDesc.pInputElementDescs = &inputElementDesc[0];

		//Fill in the pipeline description
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = { 0 };
		pipelineStateDesc.InputLayout = inputLayoutDesc;
		pipelineStateDesc.pRootSignature = signature;
		pipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(found->second.blobs[0].Get());
		pipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(found->second.blobs[1].Get());	
		if(found->second.type == (VS | GS | PS))
			pipelineStateDesc.GS = CD3DX12_SHADER_BYTECODE(found->second.blobs[2].Get());
		pipelineStateDesc.PrimitiveTopologyType = topologyType;
		pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		pipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		pipelineStateDesc.SampleDesc.Count = 1;
		pipelineStateDesc.SampleDesc.Quality = 0;
		pipelineStateDesc.SampleMask = 0xffffffff;
		pipelineStateDesc.RasterizerState = rasterDesc;
		pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		pipelineStateDesc.NumRenderTargets = 1;
		pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

		//Create a pipeline state object from the description
		assert(!m_device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(found->second.pipelineState.GetAddressOf())));

		//Release the blobs
		if (found->second.type == (VS | GS | PS))
		{
			found->second.blobs[0].Reset();
			found->second.blobs[1].Reset();
			found->second.blobs[2].Reset();
		}
	}

	void Shader::CreatePipelineStateForComputeShader(const Shaders::ID & id, ID3D12RootSignature * signature)
	{
		auto found = m_shaders.find(id);

		//Fill in compute pipeline description
		D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateDesc = { 0 };
		pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		pipelineStateDesc.CS = CD3DX12_SHADER_BYTECODE(found->second.blobs[0].Get());
		pipelineStateDesc.pRootSignature = signature;

		//Create a pipeline state object from the description
		assert(!m_device->CreateComputePipelineState(&pipelineStateDesc, IID_PPV_ARGS(found->second.pipelineState.GetAddressOf())));

		//Release blob
		found->second.blobs[0].Reset();
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
		auto found = m_shaders.find(id);
		assert(found != m_shaders.end());
		return found->second;
	}
}
