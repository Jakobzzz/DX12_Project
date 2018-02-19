#pragma once
#include <DirectXMath.h>
#include <memory>
#include <graphics/Buffer.hpp>
#include <graphics/Camera.hpp>
#include <graphics/DescriptorHeap.hpp>
#include <graphics/RootSignature.hpp>
#include <graphics/Shader.hpp>

#define NUM_BODIES 10000U
#define POINT_SIZE 1.0f

struct BodyData
{
	Vector4 position;
	Vector4 velocity;
};

namespace dx
{
	class NBodySystemCS
	{
	public:
		NBodySystemCS(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, Buffer* buffer, Camera* camera);
		void InitializeBodies();
		void UpdateBodies(Shader* shader, RootSignature* signature, float dt, const UINT & frameIndex);
		void RenderBodies(Shader* shader, RootSignature* signature, const UINT & frameIndex);

	private:
		void Initialize();

	private:
		float m_clusterScale = 1.54f;
		float m_velocityScale = 8.0f;
		UINT m_readBuffer;

	private:
		Camera * m_camera;
		Buffer * m_buffer;
		ID3D12Device * m_device;
		ID3D12GraphicsCommandList* m_commandList;

	private:
		//Constant buffers
		ComPtr<ID3D12Resource> m_cbDrawUploadHeap[2];
		ComPtr<ID3D12Resource> m_cbImmutableUploadHeap[2];
		ComPtr<ID3D12Resource> m_cbUpdateUploadHeap[2];
		UINT8* m_cbDrawAddress[2];
		UINT8* m_cbUpdateAddress[2];
		UINT8* m_cbImmutableAddress[2];

		//SRV buffer
		ComPtr<ID3D12Resource> m_srvBuffer;
		ComPtr<ID3D12Resource> m_srvBufferUploadHeap;

		//UAV buffer
		ComPtr<ID3D12Resource> m_uavBuffer;
		ComPtr<ID3D12Resource> m_uavBufferUploadHeap;

		//Descriptor heap
		std::unique_ptr<DescriptorHeap> m_srvUavDescHeap;
	};
}