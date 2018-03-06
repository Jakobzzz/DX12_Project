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
#include <utils/StepTimer.h>
#include <utils/Utility.hpp>
#include <array>
#include <D3D12Timer.hpp>

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
		void CreateTimerResources();
		void CreateComputeTimerResources();
		void CreateCommandsAndSwapChain(HWND hwnd);
		void CreateViewportAndScissorRect();
		void BeginScene(const FLOAT* color);
		void EndScene();
		void ExecuteCommandList();
		void MeasureQueueTime();
		void ComputeMeasureQueueTime();
		void ExecuteComputeCommandList();
		void WaitForGraphicsPipeline();
		void WaitForComputeShader();

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
		std::unique_ptr<StepTimer> m_fpsTimer;

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
		//Timing queries for Render
		ComPtr<ID3D12QueryHeap> m_timeQueryHeap;
		ComPtr<ID3D12Resource> m_timeQueryReadbackBuffer[FRAME_BUFFERS];
		UINT64 m_queryResults[FRAME_BUFFERS];
		UINT64 m_frequency;
		int m_queryReadbackIndex;
		int m_frameTimeEntryCount;
		int m_frameTimeNextEntry;
		std::array<double, 64> m_frameTimes;
		double m_averageDiffMs;

		//Timing queries for Compute
		ComPtr<ID3D12QueryHeap> m_computeTimeQueryHeap;
		ComPtr<ID3D12Resource> m_computeTimeQueryReadbackBuffer[FRAME_BUFFERS];
		UINT64 m_computeQueryResults[FRAME_BUFFERS];
		UINT64 m_computeFrequency;
		int m_computeQueryReadbackIndex;
		int m_computeFrameTimeEntryCount;
		int m_computeFrameTimeNextEntry;
		std::array<double, 64> m_computeFrameTimes;
		double m_computeAvrageDiffMs;

	private:
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
		double m_offset;
	};
}