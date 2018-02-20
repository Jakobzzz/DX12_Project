#pragma once
#include <d3dx12.h>
#include <wincodec.h>
#include <map>
#include <wrl.h>

namespace Textures
{
	enum class ID
	{
		Particle
	};
}

using namespace Microsoft::WRL;

namespace dx
{
	class Texture
	{
	private:
		struct TextureData
		{
			BYTE* imageData;
			D3D12_RESOURCE_DESC textureDesc;
			ComPtr<ID3D12Resource> textureBuffer;
			ComPtr<ID3D12Resource> textureBufferUploadHeap;
			UINT imageBytesPerRow;
			UINT imageSize;
		};

	public:
		Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
		void LoadTexture(const Textures::ID & id, const std::string & filename);
		void CreateSRVFromTexture(const Textures::ID & id, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle);
		void Release();

	public:
		TextureData GetTexture(const Textures::ID & id) const;

	private:
		UINT LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC & resourceDescription, LPCWSTR filename, UINT & bytesPerRow);
		WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID & wicFormatGUID);
		DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID & wicFormatGUID);
		UINT GetDXGIFormatBitsPerPixel(DXGI_FORMAT & dxgiFormat);

	private:
		ID3D12Device * m_device;
		ID3D12GraphicsCommandList* m_commandList;
		std::map<Textures::ID, TextureData> m_textures;
	};
}