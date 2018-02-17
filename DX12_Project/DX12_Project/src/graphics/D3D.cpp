#include <graphics/D3D.hpp>
#include <graphics/SamplerStates.hpp>
#include <graphics/RootDescriptor.hpp>
#include <graphics/RootParameter.hpp>
#include <utils/Utility.hpp>
#include <utils/Input.hpp>
#include <assert.h>
#include <DirectXColors.h>

namespace dx
{
	void D3D::LoadShaders()
	{
		m_shaders->LoadShaders(Shaders::ID::Triangle, "src/res/shaders/VertexShader.hlsl", "src/res/shaders/FragmentShader.hlsl");
		m_shaders->LoadComputeShader(Shaders::ID::Compute, "src/res/shaders/ComputeShader.hlsl");
	}

	void D3D::LoadTextures()
	{
		//m_texture->LoadTexture(Textures::ID::Fatboy, "src/res/textures/fatboy.png");
		//m_texture->LoadTexture(Textures::ID::Smiley, "src/res/textures/smiley.png");
	}

	void D3D::LoadObjects()
	{
		//Standard utility
		m_shaders = std::make_unique<Shader>(m_device.Get(), m_commandList.Get());
		m_texture = std::make_unique<Texture>(m_device.Get(), m_commandList.Get());
		m_buffer = std::make_unique<Buffer>(m_device.Get(), m_commandList.Get());
		m_camera = std::make_unique<Camera>();
		m_model = std::make_unique<Model>(m_device.Get(), m_commandList.Get(), m_buffer.get(), m_camera.get());

		//Descriptor heaps
		m_srvUavDescHeap = std::make_unique<DescriptorHeap>(m_device.Get(), m_commandList.Get(), 1);
		m_depthStencilHeap = std::make_unique<DescriptorHeap>(m_device.Get(), m_commandList.Get(), 1);
		
		//Root signatures
		m_rootSignature = std::make_unique<RootSignature>(m_device.Get(), m_commandList.Get());
		m_computeRootSignature = std::make_unique<RootSignature>(m_device.Get(), m_commandList.Get());
	}

	void D3D::Initialize(HWND hwnd)
	{
		//Initialize DirectX12 functionality and input
		Input::Initialize(hwnd);
		FindAndCreateDevice();
		CreateCommandsAndSwapChain(hwnd);
		CreateRenderTargetsAndFences();
		CreateViewportAndScissorRect();

		//Prepare scene
		LoadObjects();
		LoadShaders();
		LoadTextures();

		//Desc range and root table for standard pipeline 
		RootDescriptor graphicsRootDesc;
		graphicsRootDesc.AppendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
		graphicsRootDesc.CreateRootDescTable();

		//Fill in root parameters for standard pipeline
		RootParameter rootParams;
		rootParams.AppendRootParameterCBV(0, D3D12_SHADER_VISIBILITY_ALL);
		rootParams.AppendRootParameterDescTable(graphicsRootDesc.GetRootDescTable(), D3D12_SHADER_VISIBILITY_PIXEL);

		//Create a standard root signature
		m_rootSignature->CreateRootSignature((UINT)rootParams.GetRootParameters().size(), 1, &rootParams.GetRootParameters()[0], &GetStandardSamplerState(),
											  D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		//--- Compute shader ---
		RootDescriptor srvRootDesc;
		srvRootDesc.AppendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
		srvRootDesc.CreateRootDescTable();

		RootDescriptor uavRootDesc;
		uavRootDesc.AppendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);
		uavRootDesc.CreateRootDescTable();

		RootParameter computeRootParams;
		computeRootParams.AppendRootParameterDescTable(srvRootDesc.GetRootDescTable(), D3D12_SHADER_VISIBILITY_ALL);
		computeRootParams.AppendRootParameterDescTable(uavRootDesc.GetRootDescTable(), D3D12_SHADER_VISIBILITY_ALL);

		m_computeRootSignature->CreateRootSignature((UINT)computeRootParams.GetRootParameters().size(), 0, &computeRootParams.GetRootParameters()[0], NULL, 
													D3D12_ROOT_SIGNATURE_FLAG_NONE);

		//Fill in input layout and pipeline states for shaders
		m_shaders->CreatePipelineStateForComputeShader(Shaders::ID::Compute, m_computeRootSignature->GetRootSignature());
		m_shaders->CreateInputLayoutAndPipelineState(Shaders::ID::Triangle, m_rootSignature->GetRootSignature());

		//Create descriptor heaps and depth stencil buffer
		m_srvUavDescHeap->CreateDescriptorHeap(3, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_depthStencilHeap->CreateDescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_buffer->CreateDepthStencilBuffer(m_depthStencilBuffer.GetAddressOf(), m_depthViewDesc, m_depthStencilHeap->GetCPUIncrementHandle(0));

		//Create the UAV buffer
		struct Color
		{
			Vector4 color;
		};

		Color uavColor;
		uavColor.color = Vector4(0.f, 0.1f, 0.f, 1.f); //Init with blue color
		m_buffer->CreateSRVForRootTable(&uavColor, sizeof(uavColor), sizeof(Color), 1, m_srvBuffer[0].GetAddressOf(), m_srvBufferUploadHeap[0].GetAddressOf(), //SRV
			m_srvUavDescHeap->GetCPUIncrementHandle(0));

		//Initialize the srv as pixel shader resource
		SetResourceBarrier(m_srvBuffer[0].GetAddressOf(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		uavColor.color = Vector4(0.f, 0.1f, 0.f, 1.f);
		m_buffer->CreateSRVForRootTable(&uavColor, sizeof(uavColor), sizeof(Color), 1, m_srvBuffer[1].GetAddressOf(), m_srvBufferUploadHeap[1].GetAddressOf(), //SRV
			m_srvUavDescHeap->GetCPUIncrementHandle(1));

		//Initialize the srv as compute shader resource
		SetResourceBarrier(m_srvBuffer[1].GetAddressOf(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		uavColor.color = Vector4(0.f, 0.0f, 0.f, 1.f);
		m_buffer->CreateUAVForRootTable(&uavColor, sizeof(uavColor), sizeof(Color), 1, m_uavBuffer.GetAddressOf(), m_uavBufferUploadHeap.GetAddressOf(), //UAV
										m_srvUavDescHeap->GetCPUIncrementHandle(2));

		//Initialize the uav as compute shader resource
		SetResourceBarrier(m_uavBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		//Close the command list
		ExecuteCommandList();
		WaitForPreviousFrame();

		m_frameIndex = 1;
	}

	void D3D::Render()
	{
		BeginScene(Colors::DarkGray);
	
		//Set resources and draw model
		m_shaders->SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_model->BindBuffers(0, m_frameIndex);
		
		m_srvUavDescHeap->SetRootDescriptorTable(1, m_srvUavDescHeap->GetGPUIncrementHandle(m_frameIndex));
		m_model->Draw();

		EndScene();
	}

	void D3D::Simulate()
	{
		//Set compute shader to change the color of the UAV
		m_commandList->SetPipelineState(m_shaders->GetComputeShader(Shaders::ID::Compute).pipelineState.Get());
		m_computeRootSignature->SetComputeRootSignature();
		m_srvUavDescHeap->SetComputeRootDescriptorTable(0, m_srvUavDescHeap->GetGPUIncrementHandle(m_frameIndex));
		m_srvUavDescHeap->SetComputeRootDescriptorTable(1, m_srvUavDescHeap->GetGPUIncrementHandle(2));
		m_shaders->SetComputeDispatch(1, 1, 1);

		//Set the srv as a copy destination resource
		SetResourceBarrier(m_srvBuffer[m_frameIndex].GetAddressOf(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

		//Set the uav as a copy source resource
		SetResourceBarrier(m_uavBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

		//Copy the uav data to the srv
		m_commandList->CopyResource(m_srvBuffer[m_frameIndex].Get(), m_uavBuffer.Get());

		//Set the srv as a pixel shader resource
		SetResourceBarrier(m_srvBuffer[m_frameIndex].GetAddressOf(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		//Set the uav as a compute shader resource
		SetResourceBarrier(m_uavBuffer.GetAddressOf(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	}

	void D3D::BeginScene(const FLOAT* color)
	{
		//Update the input and camera
		Input::Update();
		m_camera->Update(0.00001f);

		//Exit application
		if (Input::GetKeyDown(Keyboard::Keys::Escape))
			PostQuitMessage(0);

		//Reset resources
		assert(!m_commandAllocator->Reset());
		assert(!m_commandList->Reset(m_commandAllocator.Get(), nullptr));

		//Run the compute shader
		Simulate();

		//Get the current back buffer
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

		//Set required states
		m_commandList->SetPipelineState(m_shaders->GetShaders(Shaders::ID::Triangle).pipelineState.Get());
		m_rootSignature->SetRootSignature();
		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->RSSetScissorRects(1, &m_rect);

		//Indicate that the backbuffer will be used as a render target
		CD3DX12_RESOURCE_BARRIER barrier = {};
		barrier = barrier.Transition(m_backBufferRenderTarget[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_commandList->ResourceBarrier(1, &barrier);

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

		//Set the pixel shader srv to a srv to be used by the compute shader
		SetResourceBarrier(m_srvBuffer[m_frameIndex].GetAddressOf(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		ExecuteCommandList();
		assert(!m_swapChain->Present(0, 0));

		WaitForPreviousFrame();
	}

	void D3D::ShutDown()
	{
		//Close the object handle to the fence event.
		WaitForPreviousFrame();
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

		ComPtr<ID3D12Debug1> spDebugController1;
		debugController->QueryInterface(IID_PPV_ARGS(&spDebugController1));
		spDebugController1->SetEnableGPUBasedValidation(true);
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
			result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), (void**)m_device.GetAddressOf());
			if (SUCCEEDED(result))
				break;
		}

		if (adapter)
		{
			//Create the Direct3D 12 device
			result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), (void**)m_device.GetAddressOf());
			if (FAILED(result))
				return false;
		}
		else
		{
			//Create a Direct 3D 12 device with feature level 11.1
			result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), (void**)m_device.GetAddressOf());
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

		//Create an event object for the fence.
		assert(!m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf())));
		m_fenceValue = 1;
		m_fenceEvent = CreateEvent(nullptr, 0, 0, nullptr);
	}

	void D3D::CreateCommandsAndSwapChain(HWND hwnd)
	{
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
		ZeroMemory(&commandQueueDesc, sizeof(commandQueueDesc));
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.NodeMask = 0;

		//Create command queue, allocator and list
		assert(!m_device->CreateCommandQueue(&commandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)m_commandQueue.GetAddressOf()));
		assert(!m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.GetAddressOf())));
		assert(!m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(m_commandList.GetAddressOf())));

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
		scDesc.Flags = 0;
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

	void D3D::WaitForPreviousFrame()
	{
		//Signal and increment the fence value.
		const UINT64 fence = m_fenceValue;
		m_commandQueue->Signal(m_fence.Get(), fence);
		m_fenceValue++;

		//Wait until command queue is done.
		if (m_fence->GetCompletedValue() < fence)
		{
			m_fence->SetEventOnCompletion(fence, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	void D3D::SetResourceBarrier(ID3D12Resource ** buffer, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
	{
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(*buffer, beforeState,
			afterState));
	}

	void D3D::ExecuteCommandList()
	{
		//Load command list and execute the recorded commands
		assert(!m_commandList->Close());
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
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

