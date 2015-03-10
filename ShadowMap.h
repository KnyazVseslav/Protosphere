#ifndef S_MAP_H
#define S_MAP_H

#include "D3D.h"

class ShadowMap
{

private:

	ID3D11Texture2D * depthMap;
	D3D11_VIEWPORT VP;

	ID3D11DepthStencilView * DSV; // depth map DSV
	ID3D11ShaderResourceView * SRV; // depth map SRV
	ID3D11RenderTargetView* RTV;  // depth map RTV

private:	

	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dDeviceContext;

public:

	//ShadowMap(): depthMap(0), d3dDevice(0), d3dDeviceContext(0), DSV(0), SRV(0) { ZeroMemory(&VP, sizeof(D3D11_VIEWPORT)); }
	ShadowMap() : depthMap(0), d3dDevice(0), d3dDeviceContext(0), DSV(0), SRV(0) {  }
	~ShadowMap(){ ReleaseCOM(DSV); ReleaseCOM(SRV); ReleaseCOM(RTV); }


public:


	//
	// Common methods
	//


	void Initialize(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext)
	{d3dDevice= Device; d3dDeviceContext= DeviceContext;}

	void GenerateMipsForSRV();

	ID3D11ShaderResourceView *  Get_SRV();
	ID3D11RenderTargetView *  Get_RTV();


public:


	//
	// Implementation without using RTV
	//


	void create_DSV_SRV_VP_for_depthMap(
									 UINT Width = 2048,
									 UINT Height = 2048,
									 UINT MipLevels = 1,
									 UINT ArraySize = 1,
								  	 UINT MultisamplingCount = 1,
									 UINT MultisamplingQuality = 0,
									 DXGI_FORMAT Format = DXGI_FORMAT_R24G8_TYPELESS, // If texture is intended to be interpreted in several different ways, specify DXGI_FORMAT_R32_TYPELESS
									 D3D11_USAGE Usage = D3D11_USAGE_DEFAULT, // Specify D3D11_USAGE_DYNAMIC usage if the application (CPU) needs to update the data contents of the resource frequently (e.g., on a per-frame basis). A resource with this usage can be read by the GPU and written to by the CPU.
									 UINT CPUAccessFlags = 0, //Specifies how the CPU will access the resource - read/write. Connected with prev. param. D3D11_USAGE - see comments in definition
									 UINT BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE, // flags, specifying where the resource will be bound to the pipeline
									 UINT MiscFlags = 0 // Optional flags, which do not apply to the depth/stencil buffer, so set to 0
										 );



	void OMbind_DSV_nullRTV_VP();




public:


	//
	// Implementation using RTV
	//


	void create_DSV_VP_for_depthMap(
		UINT Width = 1024,
		UINT Height = 1024,
		UINT MipLevels = 1,
		UINT ArraySize = 1,
		UINT MultisamplingCount = 1,
		UINT MultisamplingQuality = 0,
		D3D11_USAGE Usage = D3D11_USAGE_DEFAULT, // Specify D3D11_USAGE_DYNAMIC usage if the application (CPU) needs to update the data contents of the resource frequently (e.g., on a per-frame basis). A resource with this usage can be read by the GPU and written to by the CPU.
		UINT CPUAccessFlags = 0, //Specifies how the CPU will access the resource - read/write. Connected with prev. param. D3D11_USAGE - see comments in definition
		UINT MiscFlags = 0 // Optional flags, which do not apply to the depth/stencil buffer, so set to 0
		);


	void create_RTV_SRV(UINT Width = 2048,
						UINT Height = 2048,
						UINT MipLevels = 0,
						UINT ArraySize = 1,
						UINT MultisamplingCount = 1,
						UINT MultisamplingQuality = 0,
						DXGI_FORMAT Format = DXGI_FORMAT_R32_FLOAT, // If texture is intended to be interpreted in several different ways, specify DXGI_FORMAT_R32_TYPELESS
						D3D11_USAGE Usage = D3D11_USAGE_DEFAULT, // Specify D3D11_USAGE_DYNAMIC usage if the application (CPU) needs to update the data contents of the resource frequently (e.g., on a per-frame basis). A resource with this usage can be read by the GPU and written to by the CPU.
						UINT CPUAccessFlags = 0, //Specifies how the CPU will access the resource - read/write. Connected with prev. param. D3D11_USAGE - see comments in definition
						UINT BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, // flags, specifying where the resource will be bound to the pipeline
						UINT MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS // Optional flags, which do not apply to the depth/stencil buffer, so set to 0
							);	



	void OMbind_DSV_RTV_VP();



};



#endif

