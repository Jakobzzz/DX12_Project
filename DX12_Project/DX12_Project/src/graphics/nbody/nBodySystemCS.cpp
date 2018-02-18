#include <graphics/nbody/nBodySystemCS.hpp>

//Constant buffer for rendering particles
struct CB_DRAW
{
	Matrix  g_mWorldViewProjection;
	float		 g_fPointSize;
    unsigned int g_readOffset;
    float        dummy[2];
};

//Constant buffer for the simulation update compute shader
struct CB_UPDATE
{
	float g_timestep;
    float g_softeningSquared;
	unsigned int g_numParticles;
    unsigned int g_readOffset;
    unsigned int g_writeOffset;
    float dummy[3];
};

struct CB_IMMUTABLE
{
	Vector4 g_positions[4];
	//Vector4 g_texcoords[4];
};

CB_IMMUTABLE cbImmutable =
{
	Vector4(0.5f, -0.5f, 0.f, 0.f), Vector4(0.5f, 0.5f, 0.f, 0.f), Vector4(-0.5f, -0.5f, 0.f, 0.f), Vector4(-0.5f,  0.5f, 0.f, 0.f), //Positions
	//Vector4(1.f, 0.f, 0.f, 0.f), Vector4(1.f, 1.f, 0.f, 0.f), Vector4(0.f, 0.f, 0.f, 0.f), Vector4(0.f, 1.f, 0.f, 0.f), //Texcoords
};

FLOAT blendFactors[] = { 1.0f, 1.0f, 1.0f, 1.0f };

// Input layout definition
//D3D11_INPUT_ELEMENT_DESC indLayout[] =
//{
//	{ "POSINDEX", 0, DXGI_FORMAT_R32_UINT,	   0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },	
//};

namespace dx
{
	NBodySystemCS::NBodySystemCS(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, Buffer* buffer, Camera* camera) : m_device(device), 
								 m_commandList(commandList), m_buffer(buffer), m_camera(camera), m_readBuffer(0)
	{
	}

	//-----------------------------------------------------------------------------
	// Render the bodies as particles using sprites
	//-----------------------------------------------------------------------------
	//void NBodySystemCS::RenderBodies(const Matrix *p_mWorld)
	//{
	//	unsigned int particleCount = 0;

	//	D3DXMATRIX  mWorldViewProjection;
	//	D3DXMatrixMultiply(&mWorldView, p_mWorld, p_mView);
	//	D3DXMatrixMultiply(&mWorldViewProjection, &mWorldView, p_mProj);

	//	// Set the input layout
	//	m_pd3dImmediateContext->IASetInputLayout(m_pIndLayout);
	//	m_pd3dImmediateContext->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, 0);

	//	D3DXMatrixTranspose(&mWorldViewProjection, &mWorldViewProjection);

	//	// constant buffers
	//	D3D11_MAPPED_SUBRESOURCE mappedResource;
	//	V(m_pd3dImmediateContext->Map(m_pcbDraw, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	//	CB_DRAW* pcbDraw = (CB_DRAW*)mappedResource.pData;
	//	pcbDraw->g_mWorldViewProjection = mWorldViewProjection;
	//	pcbDraw->g_fPointSize = m_fPointSize;
	//	pcbDraw->g_readOffset = m_readBuffer * m_numBodies;
	//	m_pd3dImmediateContext->Unmap(m_pcbDraw, 0);

	//	particleCount = m_numBodies;
	//	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	//	m_pd3dImmediateContext->PSSetShaderResources(0, 1, &m_pParticleTexSRV);
	//	m_pd3dImmediateContext->PSSetSamplers(0, 1, &m_pParticleSamplerState);

	//	m_pd3dImmediateContext->VSSetShaderResources(0, 1, &m_pStructuredBufferSRV);

	//	m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, &m_pcbDraw);
	//	m_pd3dImmediateContext->VSSetConstantBuffers(1, 1, &m_pcbImmutable);
	//	m_pd3dImmediateContext->GSSetConstantBuffers(0, 1, &m_pcbImmutable);
	//	m_pd3dImmediateContext->PSSetConstantBuffers(0, 1, &m_pcbDraw);

	//	// Set shaders
	//	m_pd3dImmediateContext->GSSetShader(m_pGSDisplayParticle, NULL, 0);
	//	m_pd3dImmediateContext->VSSetShader(m_pVSDisplayParticleStructBuffer, NULL, 0);
	//	m_pd3dImmediateContext->PSSetShader(m_pPSDisplayParticleTex, NULL, 0);

	//	m_pd3dImmediateContext->RSSetState(m_pRasterizerState);

	//	// Set Blending State
	//	m_pd3dImmediateContext->OMSetBlendState(m_pBlendingEnabled, blendFactors, 0xFFFFFFFF);

	//	// Set Depth State
	//	m_pd3dImmediateContext->OMSetDepthStencilState(m_pDepthDisabled, 0);

	//	m_pd3dImmediateContext->Draw(particleCount, 0);

	//	// Unbound all PS shader resources
	//	ID3D11ShaderResourceView *const pSRV[4] = { NULL, NULL, NULL, NULL };
	//	m_pd3dImmediateContext->PSSetShaderResources(0, 4, pSRV);
	//	m_pd3dImmediateContext->VSSetShaderResources(0, 4, pSRV);
	//	ID3D11Buffer *const pCB[4] = { NULL, NULL, NULL, NULL };
	//	m_pd3dImmediateContext->PSSetConstantBuffers(0, 4, pCB);
	//	m_pd3dImmediateContext->GSSetConstantBuffers(0, 4, pCB);

	//	return hr;
	//}

	//-----------------------------------------------------------------------------
	// Update the positions and velocities of all bodies in the system
	// by invoking a Compute Shader that computes the gravitational force
	// on all bodies caused by all other bodies in the system.  This computation
	// is highly parallel and thus well-suited to DirectCompute shaders.
	//-----------------------------------------------------------------------------
	//void NBodySystemCS::UpdateBodies(const float & dt)
	//{
	//	// Set the time step
	//	//dt = 0.016f;//min(0.166f, max(dt, 0.033f));

	//				// constant buffers
	//	D3D11_MAPPED_SUBRESOURCE mappedResource;
	//	V(m_pd3dImmediateContext->Map(m_pcbUpdate, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	//	CB_UPDATE* pcbUpdate = (CB_UPDATE*)mappedResource.pData;
	//	pcbUpdate->g_timestep = dt;
	//	pcbUpdate->g_softeningSquared = 0.01f;
	//	pcbUpdate->g_numParticles = m_numBodies;
	//	pcbUpdate->g_readOffset = m_readBuffer * m_numBodies;
	//	pcbUpdate->g_writeOffset = (1 - m_readBuffer) * m_numBodies;
	//	m_pd3dImmediateContext->Unmap(m_pcbUpdate, 0);

	//	UINT initCounts = 0;
	//	m_pd3dImmediateContext->CSSetShader(m_pCSUpdatePositionAndVelocity, NULL, 0);
	//	m_pd3dImmediateContext->CSSetConstantBuffers(0, 1, &m_pcbUpdate);
	//	m_pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, &m_pStructuredBufferUAV, &initCounts); // CS output

	//																								   // Run the CS
	//	m_pd3dImmediateContext->Dispatch(m_numBodies / 256, 1, 1);

	//	// Unbind resources for CS
	//	ID3D11UnorderedAccessView* ppUAViewNULL[1] = { NULL };
	//	m_pd3dImmediateContext->CSSetUnorderedAccessViews(0, 1, ppUAViewNULL, &initCounts);
	//	ID3D11ShaderResourceView* ppSRVNULL[1] = { NULL };
	//	m_pd3dImmediateContext->CSSetShaderResources(0, 1, ppSRVNULL);

	//	m_readBuffer = 1 - m_readBuffer;

	//	return hr;
	//}

	//-----------------------------------------------------------------------------
	// Reset the body system to its initial configuration
	//-----------------------------------------------------------------------------
	//void NBodySystemCS::ResetBodies(BodyData configData)
	//{
	//	HRESULT hr = S_OK;

	//	m_numBodies = configData.nBodies;

	//	// for compute shader on CS_4_0, we can only have a single UAV per shader, so we have to store particle
	//	// position and velocity in the same array: all positions followed by all velocities
	//	D3DXVECTOR4 *particleArray = new D3DXVECTOR4[m_numBodies * 3];

	//	for (unsigned int i = 0; i < m_numBodies; i++) {
	//		particleArray[i] = D3DXVECTOR4(configData.position[i * 3 + 0],
	//			configData.position[i * 3 + 1],
	//			configData.position[i * 3 + 2],
	//			1.0);
	//		particleArray[i + m_numBodies] = particleArray[i];
	//		particleArray[i + 2 * m_numBodies] = D3DXVECTOR4(configData.velocity[i * 3 + 0],
	//			configData.velocity[i * 3 + 1],
	//			configData.velocity[i * 3 + 2],
	//			1.0);
	//	}

	//	D3D11_SUBRESOURCE_DATA initData = { particleArray, 0, 0 };

	//	SAFE_RELEASE(m_pStructuredBuffer);
	//	SAFE_RELEASE(m_pStructuredBufferSRV);
	//	SAFE_RELEASE(m_pStructuredBufferUAV);

	//	// Create Structured Buffer
	//	D3D11_BUFFER_DESC sbDesc;
	//	sbDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	//	sbDesc.CPUAccessFlags = 0;
	//	sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	//	sbDesc.StructureByteStride = sizeof(D3DXVECTOR4);
	//	sbDesc.ByteWidth = sizeof(D3DXVECTOR4) * m_numBodies * 3;
	//	sbDesc.Usage = D3D11_USAGE_DEFAULT;
	//	V_RETURN(m_pd3dDevice->CreateBuffer(&sbDesc, &initData, &m_pStructuredBuffer));

	//	// create the Shader Resource View (SRV) for the structured buffer
	//	D3D11_SHADER_RESOURCE_VIEW_DESC sbSRVDesc;
	//	sbSRVDesc.Buffer.ElementOffset = 0;
	//	sbSRVDesc.Buffer.ElementWidth = sizeof(D3DXVECTOR4);
	//	sbSRVDesc.Buffer.FirstElement = 0;
	//	sbSRVDesc.Buffer.NumElements = m_numBodies * 3;
	//	sbSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	//	sbSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	//	V_RETURN(m_pd3dDevice->CreateShaderResourceView(m_pStructuredBuffer, &sbSRVDesc, &m_pStructuredBufferSRV));

	//	// create the UAV for the structured buffer
	//	D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
	//	sbUAVDesc.Buffer.FirstElement = 0;
	//	sbUAVDesc.Buffer.Flags = 0;
	//	sbUAVDesc.Buffer.NumElements = m_numBodies * 3;
	//	sbUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	//	sbUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	//	V_RETURN(m_pd3dDevice->CreateUnorderedAccessView(m_pStructuredBuffer, &sbUAVDesc, &m_pStructuredBufferUAV));

	//	delete[] particleArray;
	//}

	//Create resources
	void NBodySystemCS::Initialize()
	{
		//Create constant buffers (two for normal pipeline and one for compute shader)
		m_buffer->CreateConstantBuffer(m_cbDrawUploadHeap->GetAddressOf(), &m_cbDrawAddress[0]);
		m_buffer->CreateConstantBuffer(m_cbImmutableUploadHeap->GetAddressOf(), &m_cbImmutableAddress[0]);
		m_buffer->CreateConstantBuffer(m_cbUpdateUploadHeap->GetAddressOf(), &m_cbUpdateAddress[0]);

		// rasterizer state
		//D3D11_RASTERIZER_DESC rsDesc;
		//rsDesc.FillMode = D3D11_FILL_SOLID;
		//rsDesc.CullMode = D3D11_CULL_NONE;
		//rsDesc.FrontCounterClockwise = FALSE;
		//rsDesc.DepthBias = 0;
		//rsDesc.DepthBiasClamp = 0.0f;
		//rsDesc.SlopeScaledDepthBias = 0.0f;
		//rsDesc.DepthClipEnable = TRUE;
		//rsDesc.ScissorEnable = FALSE;
		//rsDesc.MultisampleEnable = FALSE;
		//rsDesc.AntialiasedLineEnable = FALSE;
		//V_RETURN(m_pd3dDevice->CreateRasterizerState(&rsDesc, &m_pRasterizerState));

		//// Load the particle texture
		//D3DX11_IMAGE_LOAD_INFO itex;
		//D3DX11_IMAGE_INFO      pSrcInfo;

		//DXUTFindDXSDKMediaFileCch(str, MAX_PATH_STR, L"pointsprite_grey.dds");
		//hr = D3DX11GetImageInfoFromFile(str, NULL, &pSrcInfo, NULL);

		//itex.Width = pSrcInfo.Width;
		//itex.Height = pSrcInfo.Height;
		//itex.Depth = pSrcInfo.Depth;
		//itex.FirstMipLevel = 0;
		//itex.MipLevels = 8;
		//itex.Usage = D3D11_USAGE_DEFAULT;
		//itex.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D10_BIND_RENDER_TARGET;
		//itex.CpuAccessFlags = 0;
		//itex.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		//itex.Format = DXGI_FORMAT_R32_FLOAT;
		//itex.Filter = D3DX11_FILTER_LINEAR;
		//itex.MipFilter = D3DX11_FILTER_LINEAR;
		//itex.pSrcInfo = &pSrcInfo;

		//ID3D11Resource *pRes = NULL;
		//hr = D3DX11CreateTextureFromFile(m_pd3dDevice, str, &itex, NULL, &pRes, NULL);
		//if (FAILED(hr)) {
		//	MessageBox(NULL, L"Unable to create texture from file!!!", L"ERROR", MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
		//	return FALSE;
		//}

		//hr = pRes->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID*)&m_pParticleTex);
		//pRes->Release();

		//D3D11_TEXTURE2D_DESC desc;
		//m_pParticleTex->GetDesc(&desc);


		//D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;

		//SRVDesc.Format = itex.Format;
		//SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		//SRVDesc.Texture2D.MipLevels = itex.MipLevels;
		//SRVDesc.Texture2D.MostDetailedMip = 0;

		//hr |= m_pd3dDevice->CreateShaderResourceView(m_pParticleTex, &SRVDesc, &m_pParticleTexSRV);

		//m_pd3dImmediateContext->GenerateMips(m_pParticleTexSRV);

		//// Create the blending states
		//D3D11_BLEND_DESC tmpBlendState;

		//tmpBlendState.AlphaToCoverageEnable = false;
		//tmpBlendState.IndependentBlendEnable = false;

		//for (UINT i = 0; i < 8; i++) {
		//	tmpBlendState.RenderTarget[i].BlendEnable = false;
		//	tmpBlendState.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		//	tmpBlendState.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
		//	tmpBlendState.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		//	tmpBlendState.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ZERO;
		//	tmpBlendState.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		//	tmpBlendState.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		//	tmpBlendState.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		//}

		//tmpBlendState.RenderTarget[0].BlendEnable = true;

		//hr = m_pd3dDevice->CreateBlendState(&tmpBlendState, &m_pBlendingEnabled);

		//// Create the depth/stencil states
		//D3D11_DEPTH_STENCIL_DESC tmpDsDesc;
		//tmpDsDesc.DepthEnable = false;
		//tmpDsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		//tmpDsDesc.DepthFunc = D3D11_COMPARISON_LESS;
		//tmpDsDesc.StencilEnable = FALSE;

		//hr = m_pd3dDevice->CreateDepthStencilState(&tmpDsDesc, &m_pDepthDisabled);
	}
}


