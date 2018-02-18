#pragma once
#include <DirectXMath.h>
#include <memory>
#include <graphics/Buffer.hpp>
#include <graphics/Camera.hpp>

namespace dx
{
	class NBodySystemCS
	{
	public:
		struct BodyData
		{
			unsigned int nBodies;
			float* position;
			float* velocity;
		};

	public:
		NBodySystemCS(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, Buffer* buffer, Camera* camera);
		void Initialize();
		void UpdateBodies(const float & dt);  //Update the simulation
		void ResetBodies(BodyData config); //Reset the simulation
		void RenderBodies(const Matrix* world); // Render the particles

	public:
		void SetPointSize(const float & size);
		float GetPointSize() const;

	private:
		float m_fAspectRatio;
		float m_fPointSize;
		UINT m_numBodies;
		UINT m_readBuffer;

	private:
		Camera * m_camera;
		Buffer * m_buffer;
		ID3D12Device * m_device;
		ID3D12GraphicsCommandList* m_commandList;

	private:
		//Constant buffers
		ComPtr<ID3D12Resource> m_cbDrawUploadHeap[2];
		ComPtr<ID3D12Resource> m_cbUpdateUploadHeap[2];
		ComPtr<ID3D12Resource> m_cbImmutableUploadHeap[2];
		UINT8* m_cbDrawAddress[2];
		UINT8* m_cbUpdateAddress[2];
		UINT8* m_cbImmutableAddress[2];


		// Particle texture and resource views
		//ID3D11Texture2D	         *m_pParticleTex;    // Texture for displaying particles
		//   ID3D11ShaderResourceView *m_pParticleTexSRV;

		//// Vertex layout
		//ID3D11InputLayout *m_pIndLayout;

		//Render states
		//ID3D11BlendState		*m_pBlendingEnabled;
		//ID3D11SamplerState		*m_pParticleSamplerState;
		//ID3D11RasterizerState   *m_pRasterizerState;

		//// structured buffer
		//   ID3D11Texture1D           *m_pBodiesTex1D[2];
		//   ID3D11ShaderResourceView  *m_pBodiesTexSRV[2];
		//   ID3D11UnorderedAccessView *m_pBodiesTexUAV[2];
		//ID3D11Buffer			  *m_pStructuredBuffer;
		//ID3D11ShaderResourceView  *m_pStructuredBufferSRV;
		//ID3D11UnorderedAccessView *m_pStructuredBufferUAV;
	};
}