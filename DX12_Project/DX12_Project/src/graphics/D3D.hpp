#pragma once
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <dxgi1_5.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <graphics/Texture.hpp>
#include <memory>
#include <wrl.h>

using namespace DirectX;
using namespace Microsoft::WRL;

class D3D
{
public:
	void ShutDown();
	bool Initialize(HWND);
	void BeginScene(ID3D12PipelineState*);
	void EndScene();
	void ExecuteCommandList();
	void WaitForPreviousFrame();

public:
	ID3D12Device* GetDevice() const;
	ID3D12CommandQueue* GetCommandQueue() const;
	ID3D12GraphicsCommandList* GetCommandList() const;
	ID3D12RootSignature* GetRootSignature() const;

private:
	bool FindAndCreateDevice();
	void CreateRenderTargetsAndFences();
	void CreateCommandsAndSwapChain(HWND hwnd);
	void CreateViewportAndScissorRect();

private:
	struct ConstantBufferPerObject 
	{
		XMFLOAT4 color;
	};

private:
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12DescriptorHeap> m_renderTargetViewDescHeap;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<IDXGIFactory5> m_factory;

private:
	//For SRV
	std::unique_ptr<Texture> m_texture;
	ComPtr<ID3D12DescriptorHeap> m_textureDescriptorHeap;
	ComPtr<ID3D12RootSignature> m_rootSignature; 

private:
	//For constant buffer
	ComPtr<ID3D12Resource> m_constantUploadHeap[2];
	UINT8* cbvGPUAddress[2];
	ConstantBufferPerObject cbPerObject;

private:
	ComPtr<ID3D12Fence> m_fence;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12Resource> m_backBufferRenderTarget[2];

private:
	unsigned int m_frameIndex;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValue;
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_rect;
};