#pragma once
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <dxgi1_5.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <graphics/Texture.hpp>
#include <memory>
#include <graphics/Buffer.hpp>
#include <graphics/DescriptorHeap.hpp>
#include <graphics/RootSignature.hpp>
#include <graphics/Shader.hpp>

using namespace DirectX;

class D3D
{
public:
	void ShutDown();
	bool Initialize(HWND hwnd);
	void BeginScene();
	void EndScene();
	void ExecuteCommandList();
	void WaitForPreviousFrame();

public:
	ID3D12Device* GetDevice() const;
	ID3D12CommandQueue* GetCommandQueue() const;
	ID3D12GraphicsCommandList* GetCommandList() const;

private:
	void LoadShaders();
	void LoadTextures();
	void LoadObjects();

private:
	//DX12 initalizers
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
	//Objects
	std::unique_ptr<dx::Texture> m_texture;
	std::unique_ptr<dx::DescriptorHeap> m_srvDescHeap;
	std::unique_ptr<dx::RootSignature> m_rootSignature;
	std::unique_ptr<dx::Shader> m_shaders;
	std::unique_ptr<dx::Buffer> m_buffer;

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
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValue;
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_rect;
};