#pragma once
#include <DirectXMath.h>
#include <memory>
#include <graphics/Buffer.hpp>
#include <graphics/Camera.hpp>
#include <graphics/DescriptorHeap.hpp>
#include <graphics/RootSignature.hpp>
#include <graphics/Texture.hpp>
#include <graphics/Shader.hpp>
#include <utils/Utility.hpp>

//Test data values
//1024, 4096, 8192, 14336, 16384, 28672, 30720, 32768, 57344, 61440, 65536 
#define NUM_BODIES 32768

struct BodyData
{
	Vector4 position;
	Vector4 velocity;
};

namespace dx
{
	class NBody
	{
	public:
		NBody(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12GraphicsCommandList* computeCommandList, Buffer* buffer, Camera* camera, Texture* texture);
		void UpdateBodies(Shader* shader, RootSignature* signature, const UINT & frameIndex, const UINT & srvIndex);
		void RenderBodies(Shader* shader, RootSignature* signature, const UINT & frameIndex);

	private:
		void Initialize();
		void InitializeBodies();

	private:
		float m_clusterScale = 1.54f;
		float m_velocityScale = 8.0f;

	private:
		Camera * m_camera;
		Buffer * m_buffer;
		Texture * m_texture;
		ID3D12Device * m_device;
		ID3D12GraphicsCommandList* m_commandList;
		ID3D12GraphicsCommandList* m_computeCommandList;

	private:
		//Constant buffers
		ComPtr<ID3D12Resource> m_cbDrawUploadHeap[FRAME_BUFFERS];
		ComPtr<ID3D12Resource> m_cbUpdateUploadHeap[FRAME_BUFFERS];
		UINT8* m_cbDrawAddress[FRAME_BUFFERS];
		UINT8* m_cbUpdateAddress[FRAME_BUFFERS];

		//SRV buffer
		ComPtr<ID3D12Resource> m_srvBuffer[FRAME_BUFFERS];
		ComPtr<ID3D12Resource> m_srvBufferUploadHeap[FRAME_BUFFERS];

		//Descriptor heap
		std::unique_ptr<DescriptorHeap> m_srvUavDescHeap;
	};
}