#include <graphics/Shader.hpp>
#include <utils/Utility.hpp>
#include <assert.h>

void Shader::Initialize(ID3D12Device* device, ID3D12RootSignature* signature, const std::string & vertexPath, const std::string & fragPath)
{
	//Create shaders
	InitShaders(device, signature, vertexPath, fragPath);
}

void Shader::Render(ID3D12GraphicsCommandList* commandList)
{
	//Set topology
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Shader::InitShaders(ID3D12Device* device, ID3D12RootSignature* signature, const std::string & vertexPath, const std::string & fragPath)
{
	D3D12_SHADER_BYTECODE vertexShaderByteCode = {};
	D3D12_SHADER_BYTECODE pixelShaderByteCode = {};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
	ComPtr<ID3DBlob> vertexBlob;
	ComPtr<ID3DBlob> pixelBlob;

	//Create vertex and pixel shaders
	assert(!D3DCompileFromFile(ToWChar(vertexPath).c_str(), nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, vertexBlob.GetAddressOf(), nullptr));
	vertexShaderByteCode.BytecodeLength = vertexBlob->GetBufferSize();
	vertexShaderByteCode.pShaderBytecode = vertexBlob->GetBufferPointer();

	//Compile pixel shader
	assert(!D3DCompileFromFile(ToWChar(fragPath).c_str(), nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, pixelBlob.GetAddressOf(), nullptr));
	pixelShaderByteCode.BytecodeLength = pixelBlob->GetBufferSize();
	pixelShaderByteCode.pShaderBytecode = pixelBlob->GetBufferPointer();

	//Create input layout for vertex shader
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	//Get the number of elements in an array by "sizeof(array) / sizeof(arrayElementType)"
	inputLayoutDesc.NumElements = sizeof(inputElementDesc) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputElementDesc;

	//Fill in the pipeline state object desc
	pipelineStateDesc.InputLayout = inputLayoutDesc;
	pipelineStateDesc.pRootSignature = signature;
	pipelineStateDesc.VS = vertexShaderByteCode;
	pipelineStateDesc.PS = pixelShaderByteCode;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
	pipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineStateDesc.SampleDesc.Quality = 0;
	pipelineStateDesc.SampleMask = 0xffffffff;
	pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pipelineStateDesc.NumRenderTargets = 1;

	//Create a pipeline state object
	assert(!device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(m_pipelineState.GetAddressOf())));
}

ID3D12PipelineState* Shader::GetPipelineState() const
{
	return m_pipelineState.Get();
}