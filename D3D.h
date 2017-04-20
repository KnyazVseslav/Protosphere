#pragma once

#ifndef D3D_H
#define D3D_H

//d3d11.lib; dxgi.lib; d3dcompiler.lib; Facilitiesd.lib; dxerrd.lib; DDSTextureLoaderd.lib;

#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")

#include <dxgi.h>
#pragma comment (lib, "dxgi.lib")

#include <d3dcompiler.h> // for employement of D3DCompileFromFile wich will be accessible after unplugging legacy SDK, for now D3DX11CompileFromFile()
#pragma comment (lib, "d3dcompiler.lib") // for employement of D3DCompileFromFile wich will be accessible after unplugging legacy SDK, for now D3DX11CompileFromFile()



//#pragma comment (lib, "d3dx11.lib")
//#pragma comment (lib, "d3dx10.lib")



#include "modules\DDSTextureLoader\DDSTextureLoader.h"
#if defined(DEBUG) | defined(_DEBUG)
#pragma comment (lib, "modules\\DDSTextureLoader\\DDSTextureLoader_d.lib")
#else
#pragma comment (lib, "modules\\DDSTextureLoader\\DDSTextureLoader_r.lib")
#endif


#include "modules\Effects11\d3dx11effect.h"
#if defined(DEBUG) | defined(_DEBUG)
#pragma comment (lib, "modules\\Effects11\\Effects11_d.lib")
#else
#pragma comment (lib, "modules\\Effects11\\Effects11_r.lib")
#endif


#include "modules\dxerr\dxerr.h" // DXTrace()
#if defined(DEBUG) | defined(_DEBUG)
#pragma comment (lib, "modules\\dxerr\\dxerr_d.lib")
#else
#pragma comment (lib, "modules\\dxerr\\dxerr_r.lib")
#endif

//#include "query.h"


#include "modules\Facilities_LIB\Facilities_LIB.h"

#if defined(DEBUG) | defined(_DEBUG)
#pragma comment (lib, "modules\\Facilities_LIB\\Facilities_LIB_d.lib") // string_facilities::pchar_to_pwchar(char* str)
#else
#pragma comment (lib, "modules\\Facilities_LIB\\Facilities_LIB_r.lib")
#endif


//#include <D3DX11tex.h> // D3DX11CreateShaderResourceViewFromFile

#include <d3dcommon.h>

#include <vector>


#define ReleaseCOM(x) { if(x){ (x)->Release(); (x) = 0; } }
#define DeleteCOM(x) { if(x){ delete (x); (x) = 0; } }



#if defined(DEBUG) | defined(_DEBUG)
	#ifndef HR
	#define HR(x)                                              \
	{                                                          \
		HRESULT hr = (x);                                      \
		if(FAILED(hr))                                         \
		{                                                      \
			DXTrace(__FILEW__, (DWORD)__LINE__, hr, L#x, true); \
		}                                                      \
	}
	#endif

#else
	#ifndef HR
	#define HR(x) (x)
	#endif
#endif

#include <memory>


//template<class T> using sptr_of = std::shared_ptr<T>;

typedef enum Effects_Type { et_Effect11 = 0, et_ShaderSystem = 1 } Effects_Type;




using namespace std;



class D3D
{

public:

	D3D(): Device(0), DeviceContext(0),
		   SwapChain(0), RTVForSwapChainBackBuffer(0),
		   DepthStencilBuffer(0), DepthStencilView(0),
		   v_sync_enabled(0), video_card_memory_size(0)
		   
		   {
			   feature_levels_array.resize(6);

						feature_levels_array[0] = D3D_FEATURE_LEVEL_11_0;
						feature_levels_array[1] = D3D_FEATURE_LEVEL_10_1;
						feature_levels_array[2] = D3D_FEATURE_LEVEL_10_0;
						feature_levels_array[3] = D3D_FEATURE_LEVEL_9_3;
						feature_levels_array[4] = D3D_FEATURE_LEVEL_9_2;
						feature_levels_array[5] = D3D_FEATURE_LEVEL_9_1;
			   
			   /*
						feature_level_array[0] = D3D_FEATUR_LEVEL_11_1;
						feature_levels_array[1] = D3D_FEATURE_LEVEL_11_0;
						feature_levels_array[2] = D3D_FEATURE_LEVEL_10_1;
						feature_levels_array[3] = D3D_FEATURE_LEVEL_10_0;
						feature_levels_array[4] = D3D_FEATURE_LEVEL_9_3;
						feature_levels_array[5] = D3D_FEATURE_LEVEL_9_2;
						feature_levels_array[6] = D3D_FEATURE_LEVEL_9_1;

						*/
	
	
			}


	void CreateDeviceAndSwapChain(HWND RenderWindow,
									vector<D3D_FEATURE_LEVEL> feature_levels,
									BOOL Windowed = TRUE,
									bool vsync = true,
									UINT ClientWidth = 1024, 
									UINT ClientHeight = 768,
									IDXGIAdapter* DisplayAdapter = NULL, //Specifies the display adapter we want the created device to represent. Specifying null for this parameter uses the primary display adapter
									D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE,  //for test purposes set to D3D11_DRIVER_TYPE_REFERENCE 
									HMODULE Software = NULL, //set software rasterizer, use together with D3D11_DRIVER_TYPE_REFERENCE 
									UINT ReleaseDebugFlags = 0, //For release mode builds, this will generally be 0 (no extra flags); for debug mode builds, this should be D3D11_CREATE_DEVICE_DEBUG to enable the debug layer. When the debug flag is specified, Direct3D will send debug messages to the VC++ output window
									UINT SDKVersion = D3D11_SDK_VERSION,
									UINT RefreshRateNumerator = 60, 
									UINT RefreshRateDenominator = 1, 
									DXGI_FORMAT BackBufferPixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
									DXGI_MODE_SCANLINE_ORDER ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
									DXGI_MODE_SCALING Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
									UINT MultisamplingCount = 1,
									UINT MultisamplingQuality = 0,
									UINT BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
									UINT BufferCount = 1,
									DXGI_SWAP_EFFECT SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
									UINT FullScreenFlags = 0
										);

	
	void CreateRenderTargetViewForSwapChainBackBuffer();

	void CreateDepthStencilBufferAndView(UINT Width = 1024,
										 UINT Height = 768,
										 UINT MipLevels = 1,
										 UINT ArraySize = 1,
										 UINT MultisamplingCount = 1,
										 UINT MultisamplingQuality = 0,
										 DXGI_FORMAT DepthStencilTextureFormat = DXGI_FORMAT_D24_UNORM_S8_UINT, 
										 D3D11_USAGE DepthStencilTextureUsage = D3D11_USAGE_DEFAULT, // Specify D3D11_USAGE_DYNAMIC usage if the application (CPU) needs to update the data contents of the resource frequently (e.g., on a per-frame basis). A resource with this usage can be read by the GPU and written to by the CPU.
										 UINT DepthStencilTextureCPUAccessFlags = 0, //Specifies how the CPU will access the resource - read/write. Connected with prev. param. D3D11_USAGE - see comments in definition
									 	 UINT DepthStencilTextureBindFlags = D3D11_BIND_DEPTH_STENCIL, // flags, specifying where the resource will be bound to the pipeline
										 UINT DepthStencilTextureMiscFlags = 0 // Optional flags, which do not apply to the depth/stencil buffer, so set to 0
										);

	void Bind_RTV_DSV_to_OM_Pipeline_Stage();
										

	void ViewportSet(INT   ViewportTopLeftX = 0,
					 INT   ViewportTopLeftY = 0,
					 UINT  ViewportWidth = 1024,
				 	 UINT  ViewportHeight = 768,
					 FLOAT DepthBufferMinDepth = 0,
					 FLOAT DepthBufferMaxDepth = 1
					 );

	
	void Initialize(    HWND RenderWindow,
						vector<D3D_FEATURE_LEVEL> feature_levels,
						BOOL Windowed = TRUE,
						bool vsync = true,
						UINT ClientWidth = 1024, 
						UINT ClientHeight = 768,
						IDXGIAdapter* DisplayAdapter = NULL, //Specifies the display adapter we want the created device to represent. Specifying null for this parameter uses the primary display adapter
						D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE,  //for test purposes set to D3D11_DRIVER_TYPE_REFERENCE 
						HMODULE Software = NULL, //set software rasterizer, use together with D3D11_DRIVER_TYPE_REFERENCE 
						UINT ReleaseDebugFlags = D3D11_CREATE_DEVICE_DEBUG, //For release mode builds, this will generally be 0 (no extra flags); for debug mode builds, this should be D3D11_CREATE_DEVICE_DEBUG to enable the debug layer. When the debug flag is specified, Direct3D will send debug messages to the VC++ output window
						UINT SDKVersion = D3D11_SDK_VERSION,
						UINT RefreshRateNumerator = 60, 
						UINT RefreshRateDenominator = 1, 
						DXGI_FORMAT BackBufferPixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
						DXGI_MODE_SCANLINE_ORDER ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
						DXGI_MODE_SCALING Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
						UINT MultisamplingCount = 1,
						UINT MultisamplingQuality = 0,
						UINT BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
						UINT BufferCount = 1,
						DXGI_SWAP_EFFECT SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
						UINT FullScreenFlags = 0,
						DXGI_FORMAT DepthStencilTextureFormat = DXGI_FORMAT_D24_UNORM_S8_UINT, 
						D3D11_USAGE DepthStencilTextureUsage = D3D11_USAGE_DEFAULT, // Specify D3D11_USAGE_DYNAMIC usage if the application (CPU) needs to update the data contents of the resource frequently (e.g., on a per-frame basis). A resource with this usage can be read by the GPU and written to by the CPU.
						UINT DepthStencilTextureCPUAccessFlags = 0, //Specifies how the CPU will access the resource - read/write. Connected with prev. param. D3D11_USAGE - see comments in definition
						UINT DepthStencilTextureBindFlags = D3D11_BIND_DEPTH_STENCIL, // flags, specifying where the resource will be bound to the pipeline
						UINT DepthStencilTextureMiscFlags = 0, // Optional flags, which do not apply to the depth/stencil buffer, so set to 0
						INT   ViewportTopLeftX = 0,
						INT   ViewportTopLeftY = 0,
						UINT  ViewportWidth = 1024,
						UINT  ViewportHeight = 768,
						FLOAT DepthBufferMinDepth = 0,
						FLOAT DepthBufferMaxDepth = 1
						);

	void Render();

	

	void ResizeBackBuffer(UINT ClientWidth = 1024, 
						  UINT ClientHeight = 768,
						  DXGI_FORMAT BackBufferPixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM, //Set this value to DXGI_FORMAT_UNKNOWN to preserve the existing format of the back buffer
						  UINT SwapChainFlag = 0
						  );


	void Resize(UINT ClientWidth = 1024, 
				  UINT ClientHeight = 768
				  );


	void ShutDownDevice(){  if(DeviceContext) DeviceContext->ClearState(); // new

							if( SwapChain ) SwapChain->SetFullscreenState(FALSE, NULL);

							if( DepthStencilBuffer ) DepthStencilBuffer->Release();
								DepthStencilBuffer= 0;

							if( Device ) Device->Release();
								Device= 0;

							if( DeviceContext ) DeviceContext->Release();
								DeviceContext= 0;

							if( RTVForSwapChainBackBuffer ) RTVForSwapChainBackBuffer->Release();
								RTVForSwapChainBackBuffer= 0;

							if( SwapChain ) SwapChain->Release();
								SwapChain= 0;
							
						 }

	void ReleaseRTV_DSV_DSBuffer();

	void Reset_RTV_DSV_VP(UINT ClientWidth, UINT ClientHeight);

	~D3D(){ ShutDownDevice(); }

		
	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;
	IDXGISwapChain* SwapChain;

	ID3D11RenderTargetView* RTVForSwapChainBackBuffer;

	ID3D11Texture2D* DepthStencilBuffer; // буфер глубины и трафарета является 2D текстурой
	ID3D11DepthStencilView* DepthStencilView;


	vector<D3D_FEATURE_LEVEL> feature_levels_array;
	D3D_FEATURE_LEVEL feature_level;

	bool v_sync_enabled;
	int video_card_memory_size;
	char video_card_description[128];


};

#endif

