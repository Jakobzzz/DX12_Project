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
#include <graphics/Camera.hpp>
#include <graphics/nbody/nBody.hpp>
#include <utils/Utility.hpp>
#include <array>
#include <D3D12Timer.hpp>
#include <AntTweakBar.h>

using namespace DirectX;

namespace dx
{
	class D3D
	{
	public:
		void ShutDown();
		void Initialize(HWND hwnd);
		void Render();
		void Simulate();

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
		void ExecuteComputeCommandList();
		void WaitForGraphicsPipeline();
		void WaitForComputeShader();
		void CalculateComputeTime();
		void CalculateRenderTime();
		void CalculateFrameTimeAndFPS();

	private:
		std::unique_ptr<Texture> m_texture;
		std::unique_ptr<DescriptorHeap> m_depthStencilHeap;
		std::unique_ptr<RootSignature> m_rootSignature;
		std::unique_ptr<RootSignature> m_computeRootSignature;
		std::unique_ptr<Shader> m_shaders;
		std::unique_ptr<Buffer> m_buffer;
		std::unique_ptr<Camera> m_camera;
		std::unique_ptr<NBody> m_nBodySystem;
		std::unique_ptr<D3D12Timer> m_timer;
		std::unique_ptr<D3D12Timer> m_computeTimer;
		std::unique_ptr<D3D12Timer> m_fpsTimer;

	private:
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<ID3D12CommandQueue> m_computeCommandQueue;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		ComPtr<ID3D12GraphicsCommandList> m_computeCommandList;
		ComPtr<ID3D12DescriptorHeap> m_renderTargetViewDescHeap;
		ComPtr<IDXGISwapChain3> m_swapChain;
		ComPtr<IDXGIFactory5> m_factory;
		ComPtr<ID3D12Fence> m_fence;
		ComPtr<ID3D12Fence> m_computeFence;
		ComPtr<ID3D12CommandAllocator> m_computeCommandAllocator;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		ComPtr<ID3D12Resource> m_backBufferRenderTarget[FRAME_BUFFERS];
		ComPtr<ID3D12Resource> m_depthStencilBuffer;

	private:
		TwBar * m_tweakBar;
		UINT m_frameIndex;
		UINT m_srvIndex;
		HANDLE m_fenceEvent;
		HANDLE m_computeFenceEvent;
		UINT64 m_fenceValue;
		UINT64 m_fenceValues[FRAME_BUFFERS];
		UINT64 m_computeFenceValue;
		UINT64 m_computeFenceValues[FRAME_BUFFERS];
		D3D12_VIEWPORT m_viewport;
		D3D12_RECT m_rect;
		D3D12_DEPTH_STENCIL_VIEW_DESC m_depthViewDesc;
		HWND m_hwnd;
		UINT64 m_GPUCalibration;
		UINT64 m_computeGPUCalibration;
		UINT64 m_CPUCalibration;
		UINT64 m_computeCPUCalibration;
		UINT64 m_offset;
		double m_averageDiffMs = 0.0;
		double m_frame = 0.0;
		double m_overlapp = 0.0;
		int m_frameCount = 0;

		UINT64 m_freq;
		UINT64 m_computeFreq;
		LARGE_INTEGER m_cpuFreq;
		double m_sec;
		double m_computeSec;
		double m_gpuSec;
		double m_gpuComputeSec;
		double m_begin;
		double m_computeBegin;
		double m_end;
		double m_computeEnd;
		double m_3Dduration;
		double m_computeDuration;
	};
}