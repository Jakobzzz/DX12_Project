#pragma once
#include <d3d12.h>

namespace dx
{
	static D3D12_STATIC_SAMPLER_DESC GetStandardSamplerDesc()
	{
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		return sampler;
	}

	static D3D12_RASTERIZER_DESC GetNoCullRasterizerDesc()
	{
		D3D12_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.CullMode = D3D12_CULL_MODE_FRONT;
		rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;

		return rasterDesc;
	}

	static D3D12_BLEND_DESC GetParticleBlendState()
	{
		D3D12_BLEND_DESC blendDesc = { 0 };
		
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;

		for (UINT i = 0; i < 8; i++) 
		{
			blendDesc.RenderTarget[i].BlendEnable = false;
			blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}

		blendDesc.RenderTarget[0].BlendEnable = true;
		return blendDesc;
	}

}
