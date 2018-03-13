#include <graphics/D3D.hpp>
#include <graphics/CommonStates.hpp>
#include <graphics/RootDescriptor.hpp>
#include <graphics/RootParameter.hpp>
#include <utils/Utility.hpp>
#include <utils/Input.hpp>
#include <assert.h>
#include <DirectXColors.h>
#include <iostream>
#include <pix3.h>
#include <numeric>
#include <iomanip>

#ifdef min
#undef min
#endif

namespace dx
{
	void D3D::LoadShaders()
	{
		m_shaders->LoadShadersFromFile(Shaders::ID::NBody, "src/res/shaders/RenderParticles.hlsl", VS | GS | PS);
		m_shaders->LoadShadersFromFile(Shaders::ID::NBodyCompute, "src/res/shaders/nBodyCS.hlsl", CS);
	}

	void D3D::LoadTextures()
	{
		m_texture->LoadTexture(Textures::ID::Particle, "src/res/textures/star.png");
	}

	void D3D::LoadObjects()
	{
		//Standard utility
		m_shaders = std::make_unique<Shader>(m_device.Get(), m_commandList.Get(), m_computeCommandList.Get());
		m_texture = std::make_unique<Texture>(m_device.Get(), m_commandList.Get());
		m_buffer = std::make_unique<Buffer>(m_device.Get(), m_commandList.Get(), m_computeCommandList.Get());
		m_camera = std::make_unique<Camera>();
		m_timer = std::make_unique<D3D12Timer>(m_device.Get());
		m_computeTimer = std::make_unique<D3D12Timer>(m_device.Get());
		m_fpsTimer = std::make_unique<D3D12Timer>(m_device.Get());

		//Descriptor heaps
		m_depthStencilHeap = std::make_unique<DescriptorHeap>(m_device.Get(), m_commandList.Get(), m_computeCommandList.Get(), 1);
		
		//Root signatures
		m_rootSignature = std::make_unique<RootSignature>(m_device.Get(), m_commandList.Get(), m_computeCommandList.Get());
		m_computeRootSignature = std::make_unique<RootSignature>(m_device.Get(), m_commandList.Get(), m_computeCommandList.Get());
	}

	void D3D::Initialize(HWND hwnd)
	{
		//Pass the hwnd
		m_hwnd = hwnd;

		//Initialize DirectX12 functionality and input
		Input::Initialize(m_hwnd);
		FindAndCreateDevice();
		CreateCommandsAndSwapChain(m_hwnd);
		CreateRenderTargetsAndFences();
		CreateViewportAndScissorRect();

		//Prepare scene
		LoadObjects();
		LoadShaders();
		LoadTextures();

		//Init the NBody system
		m_nBodySystem = std::make_unique<NBody>(m_device.Get(), m_commandList.Get(), m_computeCommandList.Get(), m_buffer.get(), m_camera.get(), m_texture.get());

		//--- Standard shader ---
		//Desc range and root table for standard pipeline 
		RootDescriptor graphicsRootDesc;
		graphicsRootDesc.AppendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		graphicsRootDesc.AppendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

		//Fill in root parameters for standard pipeline
		RootParameter rootParams;
		rootParams.AppendRootParameterCBV(0, D3D12_SHADER_VISIBILITY_ALL);
		rootParams.AppendRootParameterDescTable(1, &graphicsRootDesc.GetDescRange()[0], D3D12_SHADER_VISIBILITY_ALL);
		rootParams.AppendRootParameterDescTable(1, &graphicsRootDesc.GetDescRange()[1], D3D12_SHADER_VISIBILITY_ALL);

		//Create a standard root signature
		m_rootSignature->CreateRootSignature((UINT)rootParams.GetRootParameters().size(), 1, &rootParams.GetRootParameters()[0], 
											  &GetStandardSamplerDesc(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		//--- Compute shader ---
		RootDescriptor uavRootDesc;
		uavRootDesc.AppendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		uavRootDesc.AppendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		uavRootDesc.CreateRootDescTable();

		RootParameter computeRootParams;
		computeRootParams.AppendRootParameterCBV(0, D3D12_SHADER_VISIBILITY_ALL);
		computeRootParams.AppendRootParameterDescTable(1, &uavRootDesc.GetDescRange()[0], D3D12_SHADER_VISIBILITY_ALL);
		computeRootParams.AppendRootParameterDescTable(1, &uavRootDesc.GetDescRange()[1], D3D12_SHADER_VISIBILITY_ALL);

		m_computeRootSignature->CreateRootSignature((UINT)computeRootParams.GetRootParameters().size(), 0, &computeRootParams.GetRootParameters()[0], nullptr, 
													D3D12_ROOT_SIGNATURE_FLAG_NONE);

		//Fill in input layout and pipeline states for shaders
		m_shaders->CreatePipelineStateForComputeShader(Shaders::ID::NBodyCompute, m_computeRootSignature->GetRootSignature());
		m_shaders->CreateInputLayoutAndPipelineState(Shaders::ID::NBody, m_rootSignature->GetRootSignature(), 
													 GetNoCullRasterizerDesc(), GetParticleBlendState(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);

		//Create descriptor heaps and depth stencil buffer
		m_depthStencilHeap->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_buffer->CreateDepthStencilBuffer(m_depthStencilBuffer.GetAddressOf(), m_depthViewDesc, m_depthStencilHeap->GetCPUIncrementHandle(0));

		//Close the command list
		ExecuteCommandList();
		ExecuteComputeCommandList();

		//Start frame index and time
		//Set the frameIndex to 1, this is to force the compute shader to start working with the next frame
		//while the graphics pipeline works with the current frame
		m_frameIndex = 1;

		//Wait for 3D queue to finish initalize
		WaitForGraphicsPipeline();

		//Wait for the compute queue to finish before we execute another
		WaitForComputeShader();

		//Set srv index so the compute shader knows which srv/uav buffer to use
		m_srvIndex = 2;

		//Get the CPU clock frequency
		QueryPerformanceFrequency(&m_cpuFreq);
	}

	void D3D::Render()
	{		
		m_fpsTimer->Tick(NULL);

		BeginScene(Colors::Black);
	
		//Set resources for normal pipeline
		m_nBodySystem->RenderBodies(m_shaders.get(), m_rootSignature.get(), m_frameIndex);

		EndScene();
	}

	void D3D::Simulate()
	{
		//Wait for the compute queue to finish before we execute another
		WaitForComputeShader();
		m_computeCommandQueue->GetClockCalibration(&m_computeGPUCalibration, &m_computeCPUCalibration);
		m_computeTimer->CalculateTime();

		m_computeCommandQueue->GetTimestampFrequency(&m_computeFreq);
		m_computeSec = m_computeCPUCalibration / (double)m_cpuFreq.QuadPart;
		m_gpuComputeSec = m_computeGPUCalibration / (double)m_computeFreq;

		m_computeBegin = m_computeTimer->GetBeginTime() / (double)m_computeFreq;
		m_computeEnd = m_computeTimer->GetEndTime() / (double)m_computeFreq;

		m_computeDuration = m_computeEnd - m_computeBegin;

		m_computeDuration -= m_computeSec;

		if (m_srvIndex == 2)
			m_srvIndex = 3;
		else
			m_srvIndex = 2;

		m_computeCommandQueue->Wait(m_fence.Get(), m_fenceValues[m_frameIndex]);

		assert(!m_computeCommandAllocator->Reset());
		assert(!m_computeCommandList->Reset(m_computeCommandAllocator.Get(), nullptr));

		m_computeTimer->Start(m_computeCommandList.Get());

		//Run the compute shader
		m_nBodySystem->UpdateBodies(m_shaders.get(), m_computeRootSignature.get(), m_frameIndex, m_srvIndex);

		m_computeTimer->Stop(m_computeCommandList.Get());
		m_computeTimer->ResolveQuery(m_computeCommandList.Get());

		ExecuteComputeCommandList();
	}

	void D3D::BeginScene(const FLOAT* color)
	{
		//Update the input and camera
		Input::Update();
		m_camera->Update(0.00001f);

		//Exit application
		if (Input::GetKeyDown(Keyboard::Keys::Escape))
			PostQuitMessage(0);

		Simulate();

		//Wait for 3D queue to finish initalize
		WaitForGraphicsPipeline();
		m_commandQueue->GetClockCalibration(&m_GPUCalibration, &m_CPUCalibration);
		m_timer->CalculateTime();

		m_freq = 0;
		m_commandQueue->GetTimestampFrequency(&m_freq);
		m_sec = m_CPUCalibration / (double)m_cpuFreq.QuadPart;
		m_gpuSec = m_GPUCalibration / (double)m_freq;
		m_begin = m_timer->GetBeginTime() / (double)m_freq;
		m_end = m_timer->GetEndTime() / (double)m_freq;

		m_3Dduration = m_end - m_begin;

		//Get the current back buffer
		//to make sure that the compute shader and graphics pipeline works on different frames
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		m_commandQueue->Wait(m_computeFence.Get(), m_computeFenceValues[m_frameIndex]);

		//Reset resourcee
		assert(!m_commandAllocator->Reset());
		assert(!m_commandList->Reset(m_commandAllocator.Get(), nullptr));

		m_timer->Start(m_commandList.Get());

		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->RSSetScissorRects(1, &m_rect);

		//Indicate that the backbuffer will be used as a render target
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_backBufferRenderTarget[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		//Get the render target view handle for the current back buffer.
		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = { 0 };
		renderTargetViewHandle = m_renderTargetViewDescHeap->GetCPUDescriptorHandleForHeapStart();
		renderTargetViewHandle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) * m_frameIndex;

		//Record commands in the command list now.
		m_commandList->OMSetRenderTargets(1, &renderTargetViewHandle, 0, &m_depthStencilHeap->GetCPUIncrementHandle(0));
		m_commandList->ClearRenderTargetView(renderTargetViewHandle, color, 0, nullptr);
		m_commandList->ClearDepthStencilView(m_depthStencilHeap->GetCPUIncrementHandle(0), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	void D3D::EndScene()
	{
		//Indicate that the backbuffer will now be used to present
		CD3DX12_RESOURCE_BARRIER barrier = {};
		barrier = barrier.Transition(m_backBufferRenderTarget[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_commandList->ResourceBarrier(1, &barrier);

		m_timer->Stop(m_commandList.Get());
		m_timer->ResolveQuery(m_commandList.Get());

		ExecuteCommandList();
			
		double diff = m_sec - m_computeSec;
		m_gpuComputeSec += diff;

		m_computeBegin += diff;
		m_computeEnd += diff;

		if (m_computeBegin < m_begin)
		{
			if (m_computeEnd > m_end)
				m_averageDiffMs = m_computeEnd - m_computeBegin;
			else
				m_averageDiffMs = m_end - m_computeBegin;
		}
		else
		{
			if (m_computeEnd > m_end)
				m_averageDiffMs = m_computeEnd - m_computeBegin;
			else
				m_averageDiffMs = m_end - m_computeBegin;
		}
		
		assert(!m_swapChain->Present(0, 0));

		if (m_frameCount < 5000)
		{
			m_frame += m_averageDiffMs;
			m_overlapp += m_computeEnd - m_begin;
		}
		
		if (m_frameCount > 5000 && m_frameCount < 5002)
		{
			std::cout << (m_frame * 1000) / 5000 << std::endl;
			std::cout << (m_overlapp * 1000) / 5000 << std::endl;
		}
		
		m_averageDiffMs *= 1000.0;

		auto titleString = std::to_string(m_averageDiffMs) + " ms (" + std::to_string(m_fpsTimer->GetFramesPerSecond()) + " FPS)";
		SetWindowTextA(m_hwnd, titleString.c_str());

		++m_frameCount;
	}

	void D3D::ShutDown()
	{
		//Close the object handle to the fence event
		WaitForGraphicsPipeline();
		WaitForComputeShader();
		CloseHandle(m_fenceEvent);

		m_texture->Release();
		m_device.Get()->Release();
	}

	bool D3D::FindAndCreateDevice()
	{
		HRESULT result;
		ComPtr<IDXGIAdapter1> adapter;

		//Find suitable device
#ifdef _DEBUG
		//Enable debug layer if in debug mode
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)debugController.GetAddressOf())))
			debugController->EnableDebugLayer();

		//Enable GPU based validation
		/*ComPtr<ID3D12Debug1> spDebugController1;
		debugController->QueryInterface(IID_PPV_ARGS(&spDebugController1));
		spDebugController1->SetEnableGPUBasedValidation(true);*/
#endif
		//Create a DirectX graphics interface factory.
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory5), (void**)m_factory.GetAddressOf());
		if (FAILED(result))
			return false;

		for (UINT adapterIndex = 0;; ++adapterIndex)
		{
			//Use the factory to create an adapter for the primary graphics interface (video card).
			result = m_factory->EnumAdapters1(adapterIndex, adapter.GetAddressOf());
			if (FAILED(result))
				break;

			//Check if a device that supports feature level 12.1 is found.
			result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);
			if (SUCCEEDED(result))
				break;
		}

		if (adapter)
		{
			//Create the Direct3D 12 device
			result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)m_device.GetAddressOf());
			if (FAILED(result))
				return false;
		}
		else
		{
			//Create a Direct 3D 12 device with feature level 11.1
			result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)m_device.GetAddressOf());
			if (FAILED(result))
				return false;
		}

		return true;
	}

	void D3D::CreateRenderTargetsAndFences()
	{
		//Initialize the frameIndex to the current back buffer index
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		//Initialize the render target view heap description for the two back buffers.
		D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc;
		ZeroMemory(&renderTargetViewHeapDesc, sizeof(renderTargetViewHeapDesc));

		//Set the number of descriptors to two for our two back buffers.  Also set the heap type to render target views.
		renderTargetViewHeapDesc.NumDescriptors = FRAME_BUFFERS;
		renderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		renderTargetViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		//Create the render target view heap for the back buffers.
		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle;
		assert(!m_device->CreateDescriptorHeap(&renderTargetViewHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)m_renderTargetViewDescHeap.GetAddressOf()));
		renderTargetViewHandle = m_renderTargetViewDescHeap->GetCPUDescriptorHandleForHeapStart();

		//Get the size of the memory location for the render target view descriptors.
		unsigned int renderTargetViewDescriptorSize;
		renderTargetViewDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		for (unsigned i = 0; i < FRAME_BUFFERS; ++i)
		{
			//Get a pointer to the back buffers from the swap chain.
			assert(!m_swapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)m_backBufferRenderTarget[i].GetAddressOf()));
			m_device->CreateRenderTargetView(m_backBufferRenderTarget[i].Get(), nullptr, renderTargetViewHandle);
			renderTargetViewHandle.ptr += renderTargetViewDescriptorSize;
		}

		//Create an event object for the compute fence
		assert(!m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_computeFence.GetAddressOf())));
		m_fenceValue = 1;
		m_computeFenceEvent = CreateEvent(nullptr, 0, 0, nullptr);

		//Create an event object for the fence.
		assert(!m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf())));
		m_fenceValue = 1;
		m_fenceEvent = CreateEvent(nullptr, 0, 0, nullptr);
	}

	void D3D::CreateCommandsAndSwapChain(HWND hwnd)
	{
		//Create compute command queue, allocator and list
		D3D12_COMMAND_QUEUE_DESC computeCommandQueueDesc;
		ZeroMemory(&computeCommandQueueDesc, sizeof(computeCommandQueueDesc));
		computeCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		computeCommandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		computeCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		computeCommandQueueDesc.NodeMask = 0;

		assert(!m_device->CreateCommandQueue(&computeCommandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)m_computeCommandQueue.GetAddressOf()));
		m_computeCommandQueue->SetName(L"Compute Command Queue");
		assert(!m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(m_computeCommandAllocator.GetAddressOf())));
		m_computeCommandAllocator->SetName(L"Compute Command Allocator");
		assert(!m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, m_computeCommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_computeCommandList.GetAddressOf())));
		m_computeCommandList->SetName(L"Compute Command List");

		D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
		ZeroMemory(&commandQueueDesc, sizeof(commandQueueDesc));
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.NodeMask = 0;

		//Create command queue, allocator and list
		assert(!m_device->CreateCommandQueue(&commandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)m_commandQueue.GetAddressOf()));
		m_commandQueue->SetName(L"Command Queue");
		assert(!m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.GetAddressOf())));
		m_commandAllocator->SetName(L"Command Allocator");
		assert(!m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(m_commandList.GetAddressOf())));
		m_commandList->SetName(L"Command List");

		//Initialize the swap chain description.
		DXGI_SWAP_CHAIN_DESC1 scDesc = {};
		scDesc.Width = SCREEN_WIDTH;
		scDesc.Height = SCREEN_HEIGHT;
		scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scDesc.Stereo = FALSE;
		scDesc.SampleDesc.Count = 1;
		scDesc.SampleDesc.Quality = 0;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.BufferCount = FRAME_BUFFERS;
		scDesc.Scaling = DXGI_SCALING_NONE;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

		//Finally create the swap chain using the swap chain description.	
		assert(!m_factory->CreateSwapChainForHwnd(m_commandQueue.Get(), hwnd, &scDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(m_swapChain.GetAddressOf())));
	}

	void D3D::CreateViewportAndScissorRect()
	{
		//Fill out the Viewport
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;
		m_viewport.Width = static_cast<float>(SCREEN_WIDTH);
		m_viewport.Height = static_cast<float>(SCREEN_HEIGHT);
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		//Fill out a scissor rect
		m_rect.left = 0;
		m_rect.top = 0;
		m_rect.right = static_cast<LONG>(SCREEN_WIDTH);
		m_rect.bottom = static_cast<LONG>(SCREEN_HEIGHT);
	}

	void D3D::WaitForGraphicsPipeline()
	{
		//Wait until command queue is done.
		if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
		{
			m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	void D3D::WaitForComputeShader()
	{
		if (m_frameIndex == 0)
		{
			if (m_computeFence->GetCompletedValue() < m_computeFenceValues[1])
			{
				m_computeFence->SetEventOnCompletion(m_computeFenceValues[1], m_computeFenceEvent);
				WaitForSingleObject(m_computeFenceEvent, INFINITE);
			}
		}
		else
		{
			if (m_computeFence->GetCompletedValue() < m_computeFenceValues[0])
			{
				m_computeFence->SetEventOnCompletion(m_computeFenceValues[0], m_computeFenceEvent);
				WaitForSingleObject(m_computeFenceEvent, INFINITE);
			}
		}
	}

	void D3D::ExecuteCommandList()
	{
		//Load command list and execute the recorded commands
		assert(!m_commandList->Close());
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//Wait for graphics pipeline
		//Signal and increment the fence value.
		m_fenceValues[m_frameIndex] = m_fenceValue;
		m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]);
		m_fenceValue++;
	}

	void D3D::ExecuteComputeCommandList()
	{
		//Load command list and execute the recorded commands
		assert(!m_computeCommandList->Close());
		ID3D12CommandList* ppCommandLists[] = { m_computeCommandList.Get() };
		m_computeCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//Wait for compute shader
		m_computeFenceValues[m_frameIndex] = m_computeFenceValue;
		m_computeCommandQueue->Signal(m_computeFence.Get(), m_computeFenceValues[m_frameIndex]);
		m_computeFenceValue++;
	}

	ID3D12Device* D3D::GetDevice() const
	{
		return m_device.Get();
	}

	ID3D12CommandQueue * D3D::GetCommandQueue() const
	{
		return m_commandQueue.Get();
	}

	ID3D12GraphicsCommandList* D3D::GetCommandList() const
	{
		return m_commandList.Get();
	}
}

