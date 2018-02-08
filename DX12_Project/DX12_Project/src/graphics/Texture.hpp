#pragma once
#include <d3d12.h>
#include <wincodec.h>
#include <map>

namespace Textures
{
	enum class ID
	{
		Dark
	};
}

class Texture
{
private:
	struct TextureData
	{
		BYTE* imageData;
		D3D12_RESOURCE_DESC textureDesc;
		ID3D12Resource* textureBuffer;
		ID3D12Resource* textureBufferUploadHeap;
		unsigned int imageBytesPerRow;
		unsigned int imageSize;
	};

public:
	Texture(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	void LoadTexture(const Textures::ID & id, const std::string & filename);
	//void AddTextureToDescriptorHeap(const Textures::ID & id, ID3D12DescriptorHeap* heap);
	void Release();

public:
	TextureData GetTexture(const Textures::ID & id) const;

private:
	unsigned int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC & resourceDescription, LPCWSTR filename, unsigned int & bytesPerRow);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID & wicFormatGUID);
	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID & wicFormatGUID);
	unsigned int GetDXGIFormatBitsPerPixel(DXGI_FORMAT & dxgiFormat);

private:
	ID3D12Device* m_device;
	ID3D12GraphicsCommandList* m_commandList;
	std::map<Textures::ID, TextureData> m_textures;
};