#include "D3D.h"

std::vector<IDXGIAdapter*> GetAdapters(IDXGIFactory* dxgiFactory, std::vector<DXGI_ADAPTER_DESC>& adaptersDescriptions)
{
	std::vector<IDXGIAdapter*> adapters;
	auto push_back_adapter = [&adapters](int idx) // позволяет передавать i-й элемент вектора для заполнения его адаптером 
	{
		adapters.resize(idx + 1);
		return &adapters[idx];
	};

	for (int idx = 0; dxgiFactory->EnumAdapters(idx, push_back_adapter(idx)) != DXGI_ERROR_NOT_FOUND; ++idx)
	{
		adaptersDescriptions.resize(idx + 1);
		adapters[idx]->GetDesc(&adaptersDescriptions[idx]);
	}

	adapters.pop_back();

	return adapters;
}

int GetArrayIdxByStr(std::vector<DXGI_ADAPTER_DESC> adaptersDescriptions, std::wstring searchStr)
{
	size_t idx = std::string::npos;

	for (const auto& desc : adaptersDescriptions)
	{
		std::wstring vendorStr = desc.Description;
		if (idx = vendorStr.find(searchStr) != std::string::npos)
			return idx;
	}

	return idx;
}

void D3D::CreateDeviceAndSwapChain(HWND RenderWindow,
									vector<D3D_FEATURE_LEVEL> feature_levels,
									BOOL Windowed,
									bool vsync, 
							 		UINT ClientWidth, 
									UINT ClientHeight,
									IDXGIAdapter* DisplayAdapter, //Specifies the display adapter we want the created device to represent. Specifying null for this parameter uses the primary display adapter
									D3D_DRIVER_TYPE DriverType,  //for test purposes set to D3D11_DRIVER_TYPE_REFERENCE 
									HMODULE Software, //set software rasterizer, use together with D3D11_DRIVER_TYPE_REFERENCE 
									UINT ReleaseDebugFlags, //For release mode builds, this will generally be 0 (no extra flags); for debug mode builds, this should be D3D11_CREATE_DEVICE_DEBUG to enable the debug layer. When the debug flag is specified, Direct3D will send debug messages to the VC++ output window
									UINT SDKVersion,
									UINT RefreshRateNumerator, 
									UINT RefreshRateDenominator, 
									DXGI_FORMAT BackBufferPixelFormat,
									DXGI_MODE_SCANLINE_ORDER ScanlineOrdering,
									DXGI_MODE_SCALING Scaling,
									UINT MultisamplingCount,
									UINT MultisamplingQuality,
									UINT BufferUsage,
									UINT BufferCount,
									DXGI_SWAP_EFFECT SwapEffect,
									UINT FullScreenFlags
									)
{

	DXGI_SWAP_CHAIN_DESC swcdesc;


	swcdesc.OutputWindow= RenderWindow;
	swcdesc.Windowed= Windowed;
	swcdesc.BufferDesc.Width= ClientWidth;
	swcdesc.BufferDesc.Height= ClientHeight;
	swcdesc.BufferDesc.RefreshRate.Numerator= RefreshRateNumerator;
	swcdesc.BufferDesc.RefreshRate.Denominator= RefreshRateDenominator;
	swcdesc.BufferDesc.Format= BackBufferPixelFormat;
	swcdesc.BufferDesc.ScanlineOrdering= ScanlineOrdering;
	swcdesc.BufferDesc.Scaling= Scaling;
	swcdesc.SampleDesc.Count= MultisamplingCount;
	swcdesc.SampleDesc.Quality= MultisamplingQuality;
	swcdesc.BufferUsage= BufferUsage;
	swcdesc.BufferCount= BufferCount;
	swcdesc.SwapEffect= SwapEffect;
	swcdesc.Flags= FullScreenFlags;
	
	
	HRESULT hr;
	if(feature_levels.size() != 0)
	{
		IDXGIFactory* dxgiFactory;
		CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
		std::vector<DXGI_ADAPTER_DESC> adaptersDescriptions;
		std::vector<IDXGIAdapter*> adapters = GetAdapters(dxgiFactory, adaptersDescriptions);
		std::wstring searchStr = L"AMD";
		int renderAdapterIdx = GetArrayIdxByStr(adaptersDescriptions, searchStr);

 	 /* HR(D3D11CreateDeviceAndSwapChain(DisplayAdapter, DriverType, Software, ReleaseDebugFlags, &feature_levels[0], feature_levels.size(), 
 	             SDKVersion, &swcdesc, &SwapChain, &Device, &feature_level, &DeviceContext));	  
				 */
		HR(D3D11CreateDeviceAndSwapChain(0, DriverType, Software, 0, &feature_levels[0], feature_levels.size(),
			SDKVersion, &swcdesc, &SwapChain, &Device, &feature_level, &DeviceContext));

		//hr = D3D11CreateDeviceAndSwapChain(adapters[renderAdapterIdx], DriverType, Software, 0, &feature_levels[0], feature_levels.size(),
			//			SDKVersion, &swcdesc, &SwapChain, &Device, &feature_level, &DeviceContext);

	}
	   else 
	   {

			HR(D3D11CreateDeviceAndSwapChain(DisplayAdapter, DriverType, Software, 0, &feature_levels_array[0], 
			          feature_levels_array.size(), SDKVersion, &swcdesc, &SwapChain, &Device, &feature_level, &DeviceContext));		

	   }

}

void D3D::CreateRenderTargetViewForSwapChainBackBuffer()
{

/*
	
	A)

			A pointer to the swap chain’s back buffer is obtained using the IDXGISwapChain::GetBuffer method. 
			The first parameter of this method is an index identifying the particular back buffer we want 
			to get (in case there is more than one). I only use one back buffer, and it has index 0. 
			
			The second parameter is the interface type of the buffer, which is usually always a 2D texture (ID3D11Texture2D). 
			
			The third parameter returns a pointer to the back buffer.

	B)
	
			To create the render target view, we use the ID3D11Device::CreateRenderTargetView method. 
			The first parameter specifies the resource that will be used as the render target, which, in this case, 
			is the back buffer.

			The second parameter is a pointer to a D3D11_RENDER_TARGET_VIEW_DESC. Among other things, 
			this structure describes the data type of the elements in the resource. If the resource was created with 
			a typed format (i.e., not typeless), then this parameter can be null, which indicates to use the format the resource 
			was created with. 
			
			The third parameter returns a pointer to the created render target view object. 

			The call to IDXGISwapChain::GetBuffer increases the COM reference count to the back buffer, which is why we release 
			it (ReleaseCOM) at the end of the code fragment.
	
*/

	// A) 


ID3D11Texture2D* SwapChainBackBuffer;

	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&SwapChainBackBuffer)); 

	// B)

	HR(Device->CreateRenderTargetView(SwapChainBackBuffer, 0, &RTVForSwapChainBackBuffer));

	//SwapChainBackBuffer->Release();
	ReleaseCOM(SwapChainBackBuffer);
	
}


void D3D::CreateDepthStencilBufferAndView(UINT Width,
										  UINT Height,
										  UINT MipLevels,
										  UINT ArraySize,
										  UINT MultisamplingCount,
										  UINT MultisamplingQuality,
										  DXGI_FORMAT DepthStencilTextureFormat, 
										  D3D11_USAGE DepthStencilTextureUsage, // Specify D3D11_USAGE_DYNAMIC usage if the application (CPU) needs to update the data contents of the resource frequently (e.g., on a per-frame basis). A resource with this usage can be read by the GPU and written to by the CPU.
										  UINT DepthStencilTextureCPUAccessFlags, //Specifies how the CPU will access the resource - read/write. Connected with prev. param. D3D11_USAGE - see comments in definition
									 	  UINT DepthStencilTextureBindFlags, // flags, specifying where the resource will be bound to the pipeline
										  UINT DepthStencilTextureMiscFlags // Optional flags, which do not apply to the depth/stencil buffer, so set to 0
										)
{


	D3D11_TEXTURE2D_DESC dsstexdesc; // описатель depth/stencil буфера

	dsstexdesc.Width= Width;
	dsstexdesc.Height= Height;
	dsstexdesc.MipLevels= 1;
	dsstexdesc.ArraySize= 1;
	dsstexdesc.Format= DepthStencilTextureFormat;
	dsstexdesc.SampleDesc.Count= MultisamplingCount; // multisampling must match swap chain values
	dsstexdesc.SampleDesc.Quality= MultisamplingQuality; // multisampling must match swap chain values
	dsstexdesc.Usage= DepthStencilTextureUsage; // Определяет как текстура будет использоваться - чтение/запись, и чем будет использоваться - CPU/GPU; D3D11_USAGE_DEFAULT - полный доступ только для GPU
	dsstexdesc.BindFlags= DepthStencilTextureBindFlags;

	

	/*
		CPUAccessFlags: Specifies how the CPU will access the resource. If the CPU needs to write to the resource, 
		specify D3D11_CPU_ACCESS_WRITE. 

		A resource with write access must have usage D3D11_USAGE_DYNAMIC or D3D11_USAGE_STAGING. 
		If the CPU needs to read from the buffer, specify D3D11_CPU_ACCESS_READ. 

		A buffer with read access must have usage D3D11_USAGE_STAGING. For the depth/stencil buffer, only the GPU writes and
		reads to the depth/buffer; therefore, we can specify 0 for this value, as the CPU will not be reading or writing to
		the depth/stencil buffer.
	*/

	dsstexdesc.CPUAccessFlags= DepthStencilTextureCPUAccessFlags;

	dsstexdesc.MiscFlags= DepthStencilTextureMiscFlags;

	Device->CreateTexture2D(&dsstexdesc, 0, &DepthStencilBuffer);
	HR(Device->CreateDepthStencilView(DepthStencilBuffer, 0, &DepthStencilView));

}


void D3D::Bind_RTV_DSV_to_OM_Pipeline_Stage()
{

	//-------------------------Привязка представлений к Output Merger стадии графического конвейера

	DeviceContext->OMSetRenderTargets(1, &RTVForSwapChainBackBuffer, DepthStencilView);

}

void D3D::ViewportSet(INT   ViewportTopLeftX,
					  INT   ViewportTopLeftY,
					  UINT  ViewportWidth,
				  	  UINT  ViewportHeight,
					  FLOAT DepthBufferMinDepth,
					  FLOAT DepthBufferMaxDepth)
{

D3D11_VIEWPORT viewport;

	viewport.TopLeftX= ViewportTopLeftX;
	viewport.TopLeftY= ViewportTopLeftY;
	viewport.Width= ViewportWidth;
	viewport.Height= ViewportHeight;

	viewport.MinDepth= DepthBufferMinDepth;
	viewport.MaxDepth= DepthBufferMaxDepth;
		
	DeviceContext->RSSetViewports(1, &viewport);

}

void D3D::Initialize(	HWND RenderWindow,
						vector<D3D_FEATURE_LEVEL> feature_levels,
						BOOL Windowed,
						bool vsync,
						UINT ClientWidth, 
						UINT ClientHeight,
						IDXGIAdapter* DisplayAdapter, //Specifies the display adapter we want the created device to represent. Specifying null for this parameter uses the primary display adapter
						D3D_DRIVER_TYPE DriverType,  //for test purposes set to D3D11_DRIVER_TYPE_REFERENCE 
						HMODULE Software, //set software rasterizer, use together with D3D11_DRIVER_TYPE_REFERENCE 
						UINT ReleaseDebugFlags, //For release mode builds, this will generally be 0 (no extra flags); for debug mode builds, this should be D3D11_CREATE_DEVICE_DEBUG to enable the debug layer. When the debug flag is specified, Direct3D will send debug messages to the VC++ output window
						UINT SDKVersion,
						UINT RefreshRateNumerator, 
						UINT RefreshRateDenominator, 
						DXGI_FORMAT BackBufferPixelFormat,
						DXGI_MODE_SCANLINE_ORDER ScanlineOrdering,
						DXGI_MODE_SCALING Scaling,
						UINT MultisamplingCount,
						UINT MultisamplingQuality,
						UINT BufferUsage,
						UINT BufferCount,
						DXGI_SWAP_EFFECT SwapEffect,
						UINT FullScreenFlags,
						DXGI_FORMAT DepthStencilTextureFormat, 
						D3D11_USAGE DepthStencilTextureUsage, // Specify D3D11_USAGE_DYNAMIC usage if the application (CPU) needs to update the data contents of the resource frequently (e.g., on a per-frame basis). A resource with this usage can be read by the GPU and written to by the CPU.
						UINT DepthStencilTextureCPUAccessFlags, //Specifies how the CPU will access the resource - read/write. Connected with prev. param. D3D11_USAGE - see comments in definition
						UINT DepthStencilTextureBindFlags, // flags, specifying where the resource will be bound to the pipeline
						UINT DepthStencilTextureMiscFlags, // Optional flags, which do not apply to the depth/stencil buffer, so set to 0
						INT   ViewportTopLeftX,
						INT   ViewportTopLeftY,
						UINT  ViewportWidth,
						UINT  ViewportHeight,
						FLOAT DepthBufferMinDepth,
						FLOAT DepthBufferMaxDepth
					)
{

	

	//-------------------------Создаём Direct3D устройство и цепь перестановок (swap chain) - ID3D11Device и IDXGISwapChain интерфейсы


	CreateDeviceAndSwapChain(RenderWindow, feature_levels, Windowed, vsync, ClientWidth, ClientHeight, DisplayAdapter, DriverType, Software, 
							  ReleaseDebugFlags, SDKVersion, RefreshRateNumerator, RefreshRateDenominator, BackBufferPixelFormat, ScanlineOrdering, 
							     Scaling, MultisamplingCount, MultisamplingQuality, BufferUsage, BufferCount, SwapEffect, FullScreenFlags);

	
	
	//-------------------------Создаём render target view для заднего буфера цепи перестановок (для бекбуфера)

	

		CreateRenderTargetViewForSwapChainBackBuffer();	

	

	
	//-------------------------Создание буфера глубины и трафарета и его представления. Привязка представления к OM стадии графического конвейера

	

	
		CreateDepthStencilBufferAndView();


			Bind_RTV_DSV_to_OM_Pipeline_Stage();

	
	//-------------------------Установка области вывода (viewport)

	ViewportSet();

}

void D3D::Render()
{

	/*
	Rendering is done in the Render() function. 
	We will render the simplest scene possible, which is to fill the screen with a single color. 
	In Direct3D 10, an easy way to fill the render target with a single color is to use the device's ClearRenderTargetView() method. 
	We first define a D3D11_COLOR structure that describes the color we would like to fill the screen with, then pass it to ClearRenderTargetView().
	In this example, a shade of blue is chosen. Once we have filled our back buffer, we call the swap chain's Present() method 
	to complete the rendering. Present() is responsible for displaying the swap chain's back buffer content onto the screen so that 
	the user can see it. 
	*/

	float R, G, B;

	R= float(53)/255;
	G= float(185)/255;
	B= float(247)/255;

	R= float(4)/255;
	G= float(251)/255;
	B= float(255)/255;

	R= float(74)/255;
	G= float(206)/255;
	B= float(255)/255;

	float bgr[4]= {R, G, B, 1.0};
		
	DeviceContext->ClearRenderTargetView(RTVForSwapChainBackBuffer, bgr);
	DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);		

}

void D3D::ResizeBackBuffer(UINT ClientWidth, 
						   UINT ClientHeight,
						   DXGI_FORMAT BackBufferPixelFormat, //Set this value to DXGI_FORMAT_UNKNOWN to preserve the existing format of the back buffer
						   UINT SwapChainFlag
						  )
{
	
// Resize the swap chain and recreate the render target view.


	HR(SwapChain->ResizeBuffers(1, ClientWidth, ClientHeight, BackBufferPixelFormat, SwapChainFlag));

		
		  
}

void D3D::ReleaseRTV_DSV_DSBuffer()
{

	ReleaseCOM(RTVForSwapChainBackBuffer);
	ReleaseCOM(DepthStencilView);
	ReleaseCOM(DepthStencilBuffer);

}

void D3D::Resize(UINT ClientWidth, 
				   UINT ClientHeight
				  )
{

	ReleaseRTV_DSV_DSBuffer();

	
	ResizeBackBuffer(ClientWidth, ClientHeight); 

	
	CreateRenderTargetViewForSwapChainBackBuffer(); // Recreate render target view for backbuffer
	

	CreateDepthStencilBufferAndView(ClientWidth, ClientHeight);
	

	Bind_RTV_DSV_to_OM_Pipeline_Stage();


	ViewportSet(0, 0, ClientWidth, ClientHeight);

}

void D3D::Reset_RTV_DSV_VP(UINT ClientWidth, UINT ClientHeight)
{

	Bind_RTV_DSV_to_OM_Pipeline_Stage();

		ViewportSet(0, 0, ClientWidth, ClientHeight);

}