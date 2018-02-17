#include <graphics/Buffer.hpp>
#include <assert.h>
#include <d3dx12.h>
#include <utils/Utility.hpp>

namespace dx
{
	Buffer::Buffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) : m_device(device), m_commandList(commandList)
	{
	}

	void Buffer::CreateVertexBuffer(const void * data, const UINT & size, const UINT & stride, ID3D12Resource ** buffer, ID3D12Resource ** uploadHeap,
									D3D12_VERTEX_BUFFER_VIEW & view)
	{
		//Create the buffer
		CreateBuffer(data, size, buffer, uploadHeap, D3D12_RESOURCE_FLAG_NONE);

		//Transition the vertex buffer data from copy destination to vertex buffer state
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer[0], D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		//Create a vertex buffer view for the primitive
		view.BufferLocation = buffer[0]->GetGPUVirtualAddress();
		view.StrideInBytes = stride;
		view.SizeInBytes = size;
	}

	void Buffer::CreateIndexBuffer(const void * data, const UINT & size, ID3D12Resource ** buffer, ID3D12Resource ** uploadHeap, 
								   D3D12_INDEX_BUFFER_VIEW & view)
	{
		//Create the buffer
		CreateBuffer(data, size, buffer, uploadHeap, D3D12_RESOURCE_FLAG_NONE);

		//Transition the index buffer data from copy destination to index buffer state
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer[0], D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_INDEX_BUFFER));

		//Create a index buffer view for the primitive
		view.BufferLocation = buffer[0]->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R32_UINT;
		view.SizeInBytes = size;
	}

	void Buffer::CreateConstantBuffer(ID3D12Resource ** buffer, UINT8 ** bufferAddress)
	{
		for (unsigned int i = 0; i < FRAME_BUFFERS; ++i)
		{
			//Create resource
			assert(!m_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE, // no flags
				&CD3DX12_RESOURCE_DESC::Buffer(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&buffer[i])));

			buffer[i]->SetName(L"Constant Buffer Upload Resource Heap");

			//Copy the data
			CD3DX12_RANGE readRange(0, 0);
			assert(!buffer[i]->Map(0, &readRange, reinterpret_cast<void**>(&bufferAddress[i])));
		}
	}

	void Buffer::CreateConstantBufferForRootTable(const UINT & size, UINT8 ** bufferAddress, ID3D12Resource ** buffer, D3D12_CPU_DESCRIPTOR_HANDLE* handlers)
	{
		for (unsigned int i = 0; i < FRAME_BUFFERS; ++i)
		{
			//Create resource
			assert(!m_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE, // no flags
				&CD3DX12_RESOURCE_DESC::Buffer(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&buffer[i])));

			buffer[i]->SetName(L"Constant Buffer Upload Resource Heap");

			D3D12_CONSTANT_BUFFER_VIEW_DESC view = { 0 };
			view.BufferLocation = buffer[0]->GetGPUVirtualAddress();
			view.SizeInBytes = (size + 255) & ~255;	//256-byte aligned CB.
			m_device->CreateConstantBufferView(&view, handlers[i]);

			//Copy the data
			CD3DX12_RANGE readRange(0, 0);
			assert(!buffer[i]->Map(0, &readRange, reinterpret_cast<void**>(&bufferAddress[i])));
		}
	}

	void Buffer::CreateUAVForRootTable(const void* data, const UINT & size, const UINT & stride, const UINT & numElements, 
										ID3D12Resource** buffer, ID3D12Resource** uploadHeap, D3D12_CPU_DESCRIPTOR_HANDLE handle)
	{
		//Create the buffer
		CreateBuffer(data, size, buffer, uploadHeap, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		//Describe the view
		D3D12_UNORDERED_ACCESS_VIEW_DESC view = {};
		view.Format = DXGI_FORMAT_UNKNOWN;
		view.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		view.Buffer.FirstElement = 0;
		view.Buffer.NumElements = numElements;
		view.Buffer.StructureByteStride = stride;
		view.Buffer.CounterOffsetInBytes = 0;
		view.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		
		//Create the UAV
		m_device->CreateUnorderedAccessView(buffer[0], nullptr, &view, handle);
	}

	void Buffer::CreateSRVForRootTable(const void * data, const UINT & size, const UINT & stride, const UINT & numElements, ID3D12Resource ** buffer, ID3D12Resource ** uploadHeap, 
										D3D12_CPU_DESCRIPTOR_HANDLE handle, D3D12_RESOURCE_STATES resourceState)
	{
		//Create the buffer
		CreateBuffer(data, size, buffer, uploadHeap, D3D12_RESOURCE_FLAG_NONE);

		//Describe the view
		D3D12_SHADER_RESOURCE_VIEW_DESC view = {};
		view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		view.Format = DXGI_FORMAT_UNKNOWN;
		view.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		view.Buffer.FirstElement = 0;
		view.Buffer.NumElements = numElements;
		view.Buffer.StructureByteStride = stride;
		view.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		//Create the SRV
		m_device->CreateShaderResourceView(buffer[0], &view, handle);
	}

	void Buffer::CreateDepthStencilBuffer(ID3D12Resource ** buffer, D3D12_DEPTH_STENCIL_VIEW_DESC & view, D3D12_CPU_DESCRIPTOR_HANDLE handle)
	{
		view.Format = DXGI_FORMAT_D32_FLOAT;
		view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		view.Flags = D3D12_DSV_FLAG_NONE;

		CD3DX12_CLEAR_VALUE depth = { DXGI_FORMAT_D32_FLOAT, 1.f, 0 };

		//Create the default heap
		m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, SCREEN_WIDTH, SCREEN_HEIGHT, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, &depth, IID_PPV_ARGS(&buffer[0]));

		m_device->CreateDepthStencilView(buffer[0], &view, handle);
	}

	void Buffer::SetConstantBufferData(const void * data, const UINT & size, const UINT & frameIndex, UINT8 ** bufferAddress)
	{
		memcpy(bufferAddress[frameIndex], data, size);
	}

	void Buffer::BindVertexBuffer(const UINT & location, D3D12_VERTEX_BUFFER_VIEW & view)
	{
		m_commandList->IASetVertexBuffers(location, 1, &view);
	}

	void Buffer::BindIndexBuffer(D3D12_INDEX_BUFFER_VIEW & view)
	{
		m_commandList->IASetIndexBuffer(&view);
	}

	void Buffer::BindConstantBufferForRootDescriptor(const UINT & rootIndex, const UINT & frameIndex, ID3D12Resource** buffer)
	{
		m_commandList->SetGraphicsRootConstantBufferView(rootIndex, buffer[frameIndex]->GetGPUVirtualAddress());
	}

	void Buffer::CreateBuffer(const void * data, const UINT & size, ID3D12Resource ** buffer, ID3D12Resource ** uploadHeap, D3D12_RESOURCE_FLAGS flags)
	{
		//Create default heap for buffer
		assert(!m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(size, flags), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer[0])));
		buffer[0]->SetName(L"Buffer Resource Heap");

		//Create the upload heap
		assert(!m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(size), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadHeap[0])));
		uploadHeap[0]->SetName(L"Buffer Upload Resource Heap");

		//Store buffer in upload heap
		D3D12_SUBRESOURCE_DATA dataType = { 0 };
		dataType.pData = data;
		dataType.RowPitch = size;
		dataType.SlicePitch = size;

		//Copy data from upload heap to default heap
		UpdateSubresources(m_commandList, buffer[0], uploadHeap[0], 0, 0, 1, &dataType);
	}
}