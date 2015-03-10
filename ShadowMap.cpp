#include "ShadowMap.h"

#include <DirectXMath.h>



void ShadowMap::GenerateMipsForSRV()
{
	d3dDeviceContext->GenerateMips(SRV);
}


ID3D11ShaderResourceView * ShadowMap::Get_SRV()
{
	return SRV;
}

ID3D11RenderTargetView * ShadowMap::Get_RTV()
{
	return RTV;
}


void ShadowMap::create_DSV_SRV_VP_for_depthMap(
	UINT Width,
	UINT Height,
	UINT MipLevels,
	UINT ArraySize,
	UINT MultisamplingCount,
	UINT MultisamplingQuality,
	DXGI_FORMAT Format, // If texture is intended to be interpreted in several different ways, specify DXGI_FORMAT_R32_TYPELESS - use in conjunction with BindFlags
	D3D11_USAGE Usage, // Specify D3D11_USAGE_DYNAMIC usage if the application (CPU) needs to update the data contents of the resource frequently (e.g., on a per-frame basis). A resource with this usage can be read by the GPU and written to by the CPU.
	UINT CPUAccessFlags, //Specifies how the CPU will access the resource - read/write. Connected with prev. param. D3D11_USAGE - see comments in definition
	UINT BindFlags, // flags, specifying where the resource will be bound to the pipeline (in which ways it'll be interpreted) - use in conjunction with Format
	UINT MiscFlags // Optional flags, which do not apply to the depth/stencil buffer, so set to 0
	)
{


	//
	// Создание depthMap как двумерной текстуры (ID3D11Texture2D * depthMap)
	//


	// описатель depth/stencil буфера
	D3D11_TEXTURE2D_DESC texdesc;

	texdesc.Width = Width;
	texdesc.Height = Height;
	texdesc.MipLevels = MipLevels;
	texdesc.ArraySize = ArraySize;
	texdesc.Format = Format; // If texture is intended to be interpreted in several different ways, specify DXGI_FORMAT_R32_TYPELESS - use in conjunction with BindFlags
	texdesc.SampleDesc.Count = MultisamplingCount; // multisampling must match swap chain values
	texdesc.SampleDesc.Quality = MultisamplingQuality; // multisampling must match swap chain values
	texdesc.Usage = Usage; // Определяет как текстура будет использоваться - чтение/запись, и чем будет использоваться - CPU/GPU; D3D11_USAGE_DEFAULT - полный доступ только для GPU

	texdesc.BindFlags = BindFlags; // flags, specifying where the resource will be bound to the pipeline (in which ways it'll be interpreted) - use in conjunction with Format

	//texdesc.BindFlags= D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;



	/*
	CPUAccessFlags: Specifies how the CPU will access the resource. If the CPU needs to write to the resource,
	specify D3D11_CPU_ACCESS_WRITE.

	A resource with write access must have usage D3D11_USAGE_DYNAMIC or D3D11_USAGE_STAGING.
	If the CPU needs to read from the buffer, specify D3D11_CPU_ACCESS_READ.

	A buffer with read access must have usage D3D11_USAGE_STAGING. For the depth/stencil buffer, only the GPU writes and
	reads to the depth/buffer; therefore, we can specify 0 for this value, as the CPU will not be reading or writing to
	the depth/stencil buffer.
	*/

	texdesc.CPUAccessFlags = CPUAccessFlags;

	texdesc.MiscFlags = MiscFlags;

	// создание карты глубин
	HR(d3dDevice->CreateTexture2D(&texdesc, 0, &depthMap));



	// Создание DSV для depthMap

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // был DXGI_FORMAT_R32_TYPELESS, теперь интерпретируем как 32-битное вещественное число для DS-буфера
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.Flags = 0;

	HR(d3dDevice->CreateDepthStencilView(depthMap, &dsvDesc, &DSV));

	

	// Создание SRV для depthMap

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;  // был DXGI_FORMAT_R32_TYPELESS, теперь интерпретируем как 32-битное вещественное число для SRV
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texdesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	d3dDevice->CreateShaderResourceView(depthMap, &srvDesc, &SRV);
	


	// View saves a reference to the texture so we can release our reference
	ReleaseCOM(depthMap);




	VP.TopLeftX = 0;
	VP.TopLeftY = 0;
	VP.Width = Width;
	VP.Height = Height;
	VP.MinDepth = 0.0f;
	VP.MaxDepth = 1.0f;

}

void ShadowMap::OMbind_DSV_nullRTV_VP()
{
	// setting

	d3dDeviceContext->RSSetViewports(1, &VP);

	ID3D11RenderTargetView * RTVs[1] = {0}; // Нулевой RTV, т.к. отрисовка происходит только в DS-буфер

	d3dDeviceContext->OMSetRenderTargets(1, RTVs, DSV);

	


	// clearing

	/*
	3 - value to wich depth buffer will be set
	4 - value to wich stencil buffer will be set
	*/													            //3  //4	
	d3dDeviceContext->ClearDepthStencilView(DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

}



//----------------------------------




void ShadowMap::create_DSV_VP_for_depthMap(
	UINT Width,
	UINT Height,
	UINT MipLevels,
	UINT ArraySize,
	UINT MultisamplingCount,
	UINT MultisamplingQuality,
	D3D11_USAGE Usage, // Specify D3D11_USAGE_DYNAMIC usage if the application (CPU) needs to update the data contents of the resource frequently (e.g., on a per-frame basis). A resource with this usage can be read by the GPU and written to by the CPU.
	UINT CPUAccessFlags, //Specifies how the CPU will access the resource - read/write. Connected with prev. param. D3D11_USAGE - see comments in definition
	UINT MiscFlags // Optional flags, which do not apply to the depth/stencil buffer, so set to 0
	)
{


	//
	// Создание depthMap как двумерной текстуры (ID3D11Texture2D * depthMap)
	//


	// описатель depth/stencil буфера
	D3D11_TEXTURE2D_DESC texdesc;

	texdesc.Width = Width;
	texdesc.Height = Height;
	texdesc.MipLevels = MipLevels;
	texdesc.ArraySize = ArraySize;
	texdesc.Format = DXGI_FORMAT_R32_TYPELESS; // If texture is intended to be interpreted in several different ways, specify DXGI_FORMAT_R32_TYPELESS - use in conjunction with BindFlags
	texdesc.SampleDesc.Count = MultisamplingCount; // multisampling must match swap chain values
	texdesc.SampleDesc.Quality = MultisamplingQuality; // multisampling must match swap chain values
	texdesc.Usage = Usage; // Определяет как текстура будет использоваться - чтение/запись, и чем будет использоваться - CPU/GPU; D3D11_USAGE_DEFAULT - полный доступ только для GPU

	texdesc.BindFlags = D3D11_BIND_DEPTH_STENCIL; // flags, specifying where the resource will be bound to the pipeline (in which ways it'll be interpreted) - use in conjunction with Format

	//texdesc.BindFlags= D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;



	/*
	CPUAccessFlags: Specifies how the CPU will access the resource. If the CPU needs to write to the resource,
	specify D3D11_CPU_ACCESS_WRITE.

	A resource with write access must have usage D3D11_USAGE_DYNAMIC or D3D11_USAGE_STAGING.
	If the CPU needs to read from the buffer, specify D3D11_CPU_ACCESS_READ.

	A buffer with read access must have usage D3D11_USAGE_STAGING. For the depth/stencil buffer, only the GPU writes and
	reads to the depth/buffer; therefore, we can specify 0 for this value, as the CPU will not be reading or writing to
	the depth/stencil buffer.
	*/

	texdesc.CPUAccessFlags = CPUAccessFlags;

	texdesc.MiscFlags = MiscFlags;

	// создание карты глубин
	HR(d3dDevice->CreateTexture2D(&texdesc, 0, &depthMap));



	// Создание DSV для depthMap

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // был DXGI_FORMAT_R32_TYPELESS, теперь интерпретируем как 32-битное вещественное число для DS-буфера
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.Flags = 0;

	HR(d3dDevice->CreateDepthStencilView(depthMap, &dsvDesc, &DSV));


	// View saves a reference to the texture so we can release our reference
	ReleaseCOM(depthMap);




	VP.TopLeftX = 0;
	VP.TopLeftY = 0;
	VP.Width = Width;
	VP.Height = Height;
	VP.MinDepth = 0.0f;
	VP.MaxDepth = 1.0f;

}




void ShadowMap::create_RTV_SRV(
	UINT Width,
	UINT Height,
	UINT MipLevels,
	UINT ArraySize,
	UINT MultisamplingCount,
	UINT MultisamplingQuality,
	DXGI_FORMAT Format, // If texture is intended to be interpreted in several different ways, specify DXGI_FORMAT_R32_TYPELESS - use in conjunction with BindFlags
	D3D11_USAGE Usage, // Specify D3D11_USAGE_DYNAMIC usage if the application (CPU) needs to update the data contents of the resource frequently (e.g., on a per-frame basis). A resource with this usage can be read by the GPU and written to by the CPU.
	UINT CPUAccessFlags, //Specifies how the CPU will access the resource - read/write. Connected with prev. param. D3D11_USAGE - see comments in definition
	UINT BindFlags, // flags, specifying where the resource will be bound to the pipeline (in which ways it'll be interpreted) - use in conjunction with Format
	UINT MiscFlags // Optional flags, which do not apply to the depth/stencil buffer, so set to 0
	)
{

	// Создание RTV и SRV для depthMap

	ID3D11Texture2D* tex;

	D3D11_TEXTURE2D_DESC texdesc; // описатель depth/stencil буфера

	texdesc.Width = Width;
	texdesc.Height = Height;
	texdesc.MipLevels = MipLevels;
	texdesc.ArraySize = ArraySize;
	texdesc.Format = Format; // If texture is intended to be interpreted in several different ways, specify DXGI_FORMAT_R32_TYPELESS - use in conjunction with BindFlags
	texdesc.SampleDesc.Count = MultisamplingCount; // multisampling must match swap chain values
	texdesc.SampleDesc.Quality = MultisamplingQuality; // multisampling must match swap chain values
	texdesc.Usage = Usage; // Определяет как текстура будет использоваться - чтение/запись, и чем будет использоваться - CPU/GPU; D3D11_USAGE_DEFAULT - полный доступ только для GPU

	texdesc.BindFlags = BindFlags; // flags, specifying where the resource will be bound to the pipeline (in which ways it'll be interpreted) - use in conjunction with Format
	texdesc.CPUAccessFlags = CPUAccessFlags;

	texdesc.MiscFlags = MiscFlags;


	d3dDevice->CreateTexture2D(&texdesc, 0, &tex);



	d3dDevice->CreateRenderTargetView(tex, 0, &RTV);
	d3dDevice->CreateShaderResourceView(tex, 0, &SRV);



	ReleaseCOM(tex);


}


void ShadowMap::OMbind_DSV_RTV_VP()
{
	// setting

	float BLACK[4] = { 0, 0, 0, 0 };

	ID3D11RenderTargetView * RTVs[1] = { RTV };

	d3dDeviceContext->OMSetRenderTargets(1, RTVs, DSV);

	d3dDeviceContext->RSSetViewports(1, &VP);


	// clearing

	d3dDeviceContext->ClearRenderTargetView(RTV, BLACK);

	/*
	3 - value to wich depth buffer will be set
	4 - value to wich stencil buffer will be set
	*/													            //3  //4	
	d3dDeviceContext->ClearDepthStencilView(DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

}

