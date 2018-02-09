#include <graphics/Buffer.hpp>
#include <assert.h>
#include <d3dx12.h>
#include <utils/Utility.hpp>

namespace dx
{
	Buffer::Buffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) : m_device(device), m_commandList(commandList)
	{
	}

	void Buffer::CreateVertexBuffer(const void * data, const unsigned int & size, const unsigned int & stride, ID3D12Resource ** buffer, ID3D12Resource ** uploadHeap,
									D3D12_VERTEX_BUFFER_VIEW & view)
	{
		//Create the buffer
		CreateBuffer(data, size, buffer, uploadHeap);

		//Transition the vertex buffer data from copy destination to vertex buffer state
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer[0], D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

		//Create a vertex buffer view for the primitive
		view.BufferLocation = buffer[0]->GetGPUVirtualAddress();
		view.StrideInBytes = stride; //Note: couldn't find a good way to get the stride from the data, should be sizeof(data[0]) but not allowed since void*...
		view.SizeInBytes = size;
	}

	void Buffer::CreateIndexBuffer(const void * data, const unsigned int & size, ID3D12Resource ** buffer, ID3D12Resource ** uploadHeap, 
								   D3D12_INDEX_BUFFER_VIEW & view)
	{
		//Create the buffer
		CreateBuffer(data, size, buffer, uploadHeap);

		//Transition the index buffer data from copy destination to index buffer state
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer[0], D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_INDEX_BUFFER));

		//Create a index buffer view for the primitive
		view.BufferLocation = buffer[0]->GetGPUVirtualAddress();
		view.Format = DXGI_FORMAT_R32_UINT;
		view.SizeInBytes = size;
	}

	void Buffer::CreateConstantBufferForRoot(const void * data, const unsigned int & size, ID3D12Resource ** buffer, UINT8 ** bufferAddress)
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
			memcpy(bufferAddress[i], data, size);
		}
	}

	void Buffer::SetConstantBufferData(const void * data, const unsigned int & size, const unsigned int & frameIndex, UINT8 ** bufferAddress)
	{
		memcpy(bufferAddress[frameIndex], data, size);
	}

	void Buffer::BindVertexBuffer(const unsigned int & location, D3D12_VERTEX_BUFFER_VIEW & view)
	{
		m_commandList->IASetVertexBuffers(location, 1, &view);
	}

	void Buffer::BindIndexBuffer(D3D12_INDEX_BUFFER_VIEW & view)
	{
		m_commandList->IASetIndexBuffer(&view);
	}

	void Buffer::BindConstantBufferForRoot(const unsigned int & rootIndex, const unsigned int & frameIndex, ID3D12Resource** buffer)
	{
		m_commandList->SetGraphicsRootConstantBufferView(rootIndex, buffer[frameIndex]->GetGPUVirtualAddress());
	}

	void Buffer::CreateBuffer(const void * data, const unsigned int & size, ID3D12Resource ** buffer, ID3D12Resource ** uploadHeap)
	{
		//Create default heap for buffer
		assert(!m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(size), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer[0])));
		buffer[0]->SetName(L"Vertex Buffer Resource Heap");

		//Create the upload heap
		assert(!m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(size), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadHeap[0])));
		uploadHeap[0]->SetName(L"Vertex Buffer Upload Resource Heap");

		//Store buffer in upload heap
		D3D12_SUBRESOURCE_DATA dataType;
		dataType.pData = data;
		dataType.RowPitch = size;
		dataType.SlicePitch = size;

		//Copy data from upload heap to default heap
		UpdateSubresources(m_commandList, buffer[0], uploadHeap[0], 0, 0, 1, &dataType);
	}
}