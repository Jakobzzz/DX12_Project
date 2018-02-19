#pragma once
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <memory>
#include <dxgi1_5.h>
#include <graphics/Texture.hpp>
#include <graphics/Buffer.hpp>
#include <graphics/DescriptorHeap.hpp>
#include <graphics/RootSignature.hpp>
#include <graphics/Shader.hpp>
#include <graphics/Model.hpp>
#include <graphics/Camera.hpp>
#include <graphics/nbody/nBody.hpp>

using namespace DirectX;

namespace dx
{
	class D3D
	{
	public:
		void ShutDown();
		void Initialize(HWND hwnd);
		void Render();

	public:
		ID3D12Device * GetDevice() const;
		ID3D12CommandQueue* GetCommandQueue() const;
		ID3D12GraphicsCommandList* GetCommandList() const;

	private:
		void LoadShaders();
		void LoadTextures();
		void LoadObjects();

	private:
		//DX12 functionality
		bool FindAndCreateDevice();
		void CreateRenderTargetsAndFences();
		void CreateCommandsAndSwapChain(HWND hwnd);
		void CreateViewportAndScissorRect();
		void BeginScene(const FLOAT* color);
		void EndScene();
		void ExecuteCommandList();
		void WaitForPreviousFrame();

	private:
		std::unique_ptr<Texture> m_texture;
		std::unique_ptr<DescriptorHeap> m_depthStencilHeap;
		std::unique_ptr<RootSignature> m_rootSignature;
		std::unique_ptr<RootSignature> m_computeRootSignature;
		std::unique_ptr<Shader> m_shaders;
		std::unique_ptr<Buffer> m_buffer;
		std::unique_ptr<Camera> m_camera;
		std::unique_ptr<Model> m_model;
		//std::unique_ptr<NBody> m_nBodySystem;

	private:
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		ComPtr<ID3D12DescriptorHeap> m_renderTargetViewDescHeap;
		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<IDXGIFactory5> m_factory;
		ComPtr<ID3D12Fence> m_fence;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		ComPtr<ID3D12Resource> m_backBufferRenderTarget[2];
		ComPtr<ID3D12Resource> m_depthStencilBuffer;

	private:
		UINT m_frameIndex;
		HANDLE m_fenceEvent;
		UINT64 m_fenceValue;
		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_rect;
		D3D12_DEPTH_STENCIL_VIEW_DESC m_depthViewDesc;
	};
}