#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <d3dcompiler.h>
#include <fstream>
#include <wrl.h>

using namespace Microsoft::WRL;

class Shader
{
public:
	void Initialize(ID3D12Device*, ID3D12RootSignature*);
	void Render(ID3D12GraphicsCommandList*);

public:
	ID3D12PipelineState* GetPipelineState() const;

private:
	void InitShaders(ID3D12Device*, WCHAR*, WCHAR*, ID3D12RootSignature*);

private:
	ComPtr<ID3D12PipelineState> m_pipelineState;
};
