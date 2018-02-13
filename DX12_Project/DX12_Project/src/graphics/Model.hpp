#pragma once
#include <DirectXMath.h>
#include <memory>
#include <graphics/Buffer.hpp>
#include <graphics/Camera.hpp>

namespace dx
{
	class Model
	{
	public:
		Model(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, Buffer* buffer, Camera* camera);

	public:
		void BindBuffers(const UINT & rootIndex, const UINT & frameIndex);
		void Draw();

	public:
		struct Vertex
		{
			XMFLOAT3 position;
			XMFLOAT2 uv;
		};

		struct CBInfo
		{
			Matrix WVP;
		};

	private:
		ComPtr<ID3D12Resource> m_vertexBuffer;
		ComPtr<ID3D12Resource> m_indexBuffer;
		ComPtr<ID3D12Resource> m_vertexBufferUploadHeap;
		ComPtr<ID3D12Resource> m_constantUploadHeap[2];
		UINT8* m_cbvGPUAddress[2];
		CBInfo m_cb;

	private:
		Matrix m_WVP;
		Matrix m_world;

	private:
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	private:
		Camera * m_camera;
		Buffer * m_buffer;
		ID3D12Device * m_device;
		ID3D12GraphicsCommandList* m_commandList;
	};
}