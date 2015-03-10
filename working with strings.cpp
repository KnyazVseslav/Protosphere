//=============================================================================
// Lighting.cpp by Frank Luna (C) 2008 All Rights Reserved.
//
// Demonstrates a directional, point, and spot light.
//
// Controls:
//		'A'/'D'/'W'/'S' - Rotate 
//              'Z'/'X' - Zoom
//          '1'/'2'/'2' - Switch light type
//
//=============================================================================

#include "d3dApp.h"
#include "PeaksAndValleys.h"
#include "Waves.h"
#include "Box.h"
#include "Light.h"
#include "Cylinder.h"

#include <sstream>


class LightingApp : public D3DApp
{
public:
	LightingApp(HINSTANCE hInstance);
	~LightingApp();

	void initApp();
	void onResize();
	void updateScene(float dt);
	void drawScene(); 

private:
	void buildFX();
	void buildVertexLayouts();

private:

	PeaksAndValleys mLand;
	Waves mWaves;

	Box mCrate;
	Cylinder cylinder;
	

	Light mLights[3];
	int mLightType; // 0 (parallel), 1 (point), 2 (spot), 3(spoint)


	ID3D10Effect* mFX;
	ID3D10EffectTechnique* mTech;
	ID3D10EffectMatrixVariable* mfxWVPVar;
	ID3D10EffectMatrixVariable* mfxWorldVar;
	ID3D10EffectMatrixVariable* mfxTexMtxVar;
	ID3D10EffectVariable* mfxEyePosVar;
	ID3D10EffectVariable* mfxLightVar;
	ID3D10EffectVariable* mfxLightVar2;	
	ID3D10EffectScalarVariable* mfxLightType;
	
	
	ID3D10EffectShaderResourceVariable* mfxDiffuseMapVar;	
	ID3D10EffectShaderResourceVariable* mfxSpecularMapVar;

	ID3D10ShaderResourceView* pGrassMap;
	ID3D10ShaderResourceView* pWavesMap;
	ID3D10ShaderResourceView* pCrateMap;
	ID3D10ShaderResourceView* pSpecularMap;

	
	ID3D10ShaderResourceView** ppLightningAnim;

	ID3D10InputLayout* mVertexLayout;
 
	D3DXMATRIX mLandWorld;
	D3DXMATRIX mCrateWorld;
	D3DXMATRIX mWavesWorld;

	D3DXMATRIX mTexMtxLand;
	D3DXMATRIX mTexMtxCrate;
	D3DXMATRIX mTexMtxWaves;

	D3DXVECTOR3 vT;


	ID3D10RasterizerState* Wireframe;
	ID3D10RasterizerState* Solid;
	ID3D10RasterizerState* CullNone;

	ID3D10BlendState* pBlendStateTransparency;
	ID3D10BlendState* pBlendStateAdditive;

	ID3D10DepthStencilState** ppDepthStencilState;

	

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
	D3DXMATRIX mWVP;

	D3DXVECTOR3 mEyePos;
	float mRadius;
	float mTheta;
	float mPhi;

	float coneAngleExponential;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif


	LightingApp theApp(hInstance);
	
	theApp.initApp();

	return theApp.run();
}

LightingApp::LightingApp(HINSTANCE hInstance)
: D3DApp(hInstance), mLightType(0), mFX(0), mTech(0), mfxWVPVar(0), mfxWorldVar(0), 
  mfxEyePosVar(0), mfxLightVar(0), mfxLightType(0), mVertexLayout(0), 
  mEyePos(0.0f, 0.0f, 0.0f), mRadius(80.0f), mTheta(0.0f), mPhi(PI*0.4f), coneAngleExponential(64.0f), vT(0, 0, 0)
{
	
	D3DXMatrixIdentity(&mLandWorld);
	D3DXMatrixIdentity(&mCrateWorld);
	D3DXMatrixIdentity(&mWavesWorld);
	D3DXMatrixIdentity(&mView);
	D3DXMatrixIdentity(&mProj);
	D3DXMatrixIdentity(&mWVP);

	D3DXMatrixScaling(&mTexMtxLand, 5, 5, 1);
	D3DXMatrixIdentity(&mTexMtxCrate);

}

LightingApp::~LightingApp()
{
	if( md3dDevice )
		md3dDevice->ClearState();

	ReleaseCOM(mFX);
	ReleaseCOM(mVertexLayout);
}

void LightingApp::initApp()
{
	D3DApp::initApp();

	mClearColor = D3DXCOLOR(0.9f, 0.9f, 0.9f, 1.0f);

	
	mLand.init(md3dDevice, 129, 129, 1.0f);
	mCrate.init(md3dDevice, 15.0f);
	mWaves.init(md3dDevice, 257, 257, 0.5f, 0.03f, 3.25f, 0.1f);
	//mWaves.init(md3dDevice, 500, 500, 0.25f, 0.03f, 3.25f, 0.0f);

	           
	
								cylinder.init(md3dDevice, 20, 2, 16, 36);

	

	HR(D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"WoodCrate01.dds", 0, 0, &pCrateMap, 0));
	HR(D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"grass.dds", 0, 0, &pGrassMap, 0));
	HR(D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"water2a.dds", 0, 0, &pWavesMap, 0));
	HR(D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"defaultspec.dds", 0, 0, &pSpecularMap, 0));



	// Loading lightning animation;
	// thirty frames per second during two seconds

				//-------String testing

						LPCWSTR str = L"1111";
						char* pstr = "23131";
						char cstr[] = "12313413";

						const WCHAR* wstr = LPCWSTR('1');
	
							std::stringstream ss;

							  ss << 1;
	
						std::string s = ss.str();
							s+= "22";
							//std::wstring(s.begin(), s.end()).c_str(); // перевод std::string в LPCWSTR
				// -------------------------------------------



	ppLightningAnim = new ID3D10ShaderResourceView* [60];

	
	std::stringstream ss_conv;
	


	//HR(D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(1).bmp", 0, 0, pppLightningAnim, 0));L"lightning("+L").bmp"
/**/
	for(UINT i = 0; i < 60; i++)
	{
		ss_conv << i+1;
		  s = "lightning("+ss_conv.str()+").bmp";
		ss_conv.str(std::string()); /* or we may have used ss_conv.str(""), but ss_conv.str(std::string()) is technically more efficient, because you
									   avoid invoking the "std::string" constructor that takes "const char*", but any compiler these days should be
									   able to generate the same code in both cases - so I would just go with whatever is more readable
									*/

		D3DX10CreateShaderResourceViewFromFile(md3dDevice, std::wstring(s.begin(), s.end()).c_str(), 0, 0, ppLightningAnim+i, 0);
	}


/*
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(1).bmp",  0, 0, ppLightningAnim, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(2).bmp",  0, 0, ppLightningAnim+1, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(3).bmp",  0, 0, ppLightningAnim+2, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(4).bmp",  0, 0, ppLightningAnim+3, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(5).bmp",  0, 0, ppLightningAnim+4, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(6).bmp",  0, 0, ppLightningAnim+5, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(7).bmp",  0, 0, ppLightningAnim+6, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(8).bmp",  0, 0, ppLightningAnim+7, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(9).bmp",  0, 0, ppLightningAnim+8, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(10).bmp", 0, 0, ppLightningAnim+9, 0);

	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(11).bmp", 0, 0, ppLightningAnim+10, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(12).bmp", 0, 0, ppLightningAnim+11, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(13).bmp", 0, 0, ppLightningAnim+12, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(14).bmp", 0, 0, ppLightningAnim+13, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(15).bmp", 0, 0, ppLightningAnim+14, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(16).bmp", 0, 0, ppLightningAnim+15, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(17).bmp", 0, 0, ppLightningAnim+16, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(18).bmp", 0, 0, ppLightningAnim+17, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(19).bmp", 0, 0, ppLightningAnim+18, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(20).bmp", 0, 0, ppLightningAnim+19, 0);

	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(21).bmp",  0, 0, ppLightningAnim+20, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(22).bmp",  0, 0, ppLightningAnim+21, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(23).bmp",  0, 0, ppLightningAnim+22, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(24).bmp",  0, 0, ppLightningAnim+23, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(25).bmp",  0, 0, ppLightningAnim+24, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(26).bmp",  0, 0, ppLightningAnim+25, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(27).bmp",  0, 0, ppLightningAnim+26, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(28).bmp",  0, 0, ppLightningAnim+27, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(29).bmp",  0, 0, ppLightningAnim+28, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(30).bmp",  0, 0, ppLightningAnim+29, 0);

	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(31).bmp",  0, 0, ppLightningAnim+30, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(32).bmp",  0, 0, ppLightningAnim+31, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(33).bmp",  0, 0, ppLightningAnim+32, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(34).bmp",  0, 0, ppLightningAnim+33, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(35).bmp",  0, 0, ppLightningAnim+34, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(36).bmp",  0, 0, ppLightningAnim+35, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(37).bmp",  0, 0, ppLightningAnim+36, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(38).bmp",  0, 0, ppLightningAnim+37, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(39).bmp",  0, 0, ppLightningAnim+38, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(40).bmp",  0, 0, ppLightningAnim+39, 0);

	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(41).bmp",  0, 0, ppLightningAnim+40, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(42).bmp",  0, 0, ppLightningAnim+41, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(43).bmp",  0, 0, ppLightningAnim+42, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(44).bmp",  0, 0, ppLightningAnim+43, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(45).bmp",  0, 0, ppLightningAnim+44, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(46).bmp",  0, 0, ppLightningAnim+45, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(47).bmp",  0, 0, ppLightningAnim+46, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(48).bmp",  0, 0, ppLightningAnim+47, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(49).bmp",  0, 0, ppLightningAnim+48, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(50).bmp",  0, 0, ppLightningAnim+49, 0);

	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(51).bmp",  0, 0, ppLightningAnim+50, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(52).bmp",  0, 0, ppLightningAnim+51, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(53).bmp",  0, 0, ppLightningAnim+52, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(54).bmp",  0, 0, ppLightningAnim+53, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(55).bmp",  0, 0, ppLightningAnim+54, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(56).bmp",  0, 0, ppLightningAnim+55, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(57).bmp",  0, 0, ppLightningAnim+56, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(58).bmp",  0, 0, ppLightningAnim+57, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(59).bmp",  0, 0, ppLightningAnim+58, 0);
	D3DX10CreateShaderResourceViewFromFile(md3dDevice, L"lightning(60).bmp",  0, 0, ppLightningAnim+59, 0);
*/


	buildFX();
	buildVertexLayouts();

	mLightType = 0;
 
	// Parallel light.
	mLights[0].dir      = D3DXVECTOR3(0.57735f, -0.57735f, 0.57735f);
	mLights[0].ambient  = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	//mLights[0].ambient  = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	mLights[0].diffuse  = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLights[0].specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
 
	// Pointlight--position is changed every frame to animate.
	mLights[1].ambient  = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	mLights[1].diffuse  = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLights[1].specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLights[1].att.x    = 0.0f;
	mLights[1].att.y    = 0.1f;
	mLights[1].att.z    = 0.0f;
	mLights[1].range    = 50.0f;

	// Spotlight--position and direction changed every frame to animate.
	mLights[2].ambient  = D3DXCOLOR(0.4f, 0.4f, 0.4f, 1.0f);
	mLights[2].diffuse  = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	mLights[2].specular = D3DXCOLOR(1.0f, 1.0f, 1.0f, 512.0f);
	mLights[2].att.x    = 1.0f;
	mLights[2].att.y    = 0.0f;
	mLights[2].att.z    = 0.0f;
	mLights[2].spotPow  = coneAngleExponential;
	mLights[2].range    = 10000.0f;


 //Rasterizer states

	//Wireframe rs

	D3D10_RASTERIZER_DESC rd;
	ZeroMemory(&rd, sizeof(D3D10_RASTERIZER_DESC));
	//rd.CullMode = D3D10_CULL_BACK;
	rd.CullMode = D3D10_CULL_NONE;
	rd.FillMode = D3D10_FILL_WIREFRAME;
	rd.FrontCounterClockwise = false;

	HR(md3dDevice->CreateRasterizerState(&rd, &Wireframe));



	//Solid rs

	ZeroMemory(&rd, sizeof(D3D10_RASTERIZER_DESC));
	rd.CullMode = D3D10_CULL_BACK;
	rd.FillMode = D3D10_FILL_SOLID;
	rd.FrontCounterClockwise = false;

	HR(md3dDevice->CreateRasterizerState(&rd, &Solid));


	//CullNone rs

	ZeroMemory(&rd, sizeof(D3D10_RASTERIZER_DESC));
	rd.CullMode = D3D10_CULL_NONE;
	rd.FillMode = D3D10_FILL_SOLID;
	rd.FrontCounterClockwise = false;

	HR(md3dDevice->CreateRasterizerState(&rd, &CullNone));

	

 //Blend state

   D3D10_BLEND_DESC bd = {0};
	//D3D10_BLEND_DESC bd;

   //transparency blending
   bd.AlphaToCoverageEnable = false;
   bd.BlendEnable[0] = true;
   bd.SrcBlend = D3D10_BLEND_SRC_ALPHA;
   bd.DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
   bd.BlendOp = D3D10_BLEND_OP_ADD;
   bd.SrcBlendAlpha = D3D10_BLEND_ONE;
   bd.DestBlendAlpha = D3D10_BLEND_ZERO;
   bd.BlendOpAlpha = D3D10_BLEND_OP_ADD;  
   bd.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;

   md3dDevice->CreateBlendState(&bd, &pBlendStateTransparency);
  

   //additive blending
   bd.AlphaToCoverageEnable = false;
   bd.BlendEnable[0] = true;
   bd.SrcBlend = D3D10_BLEND_ONE;
   bd.DestBlend = D3D10_BLEND_ONE;
   bd.BlendOp = D3D10_BLEND_OP_ADD;
   bd.SrcBlendAlpha = D3D10_BLEND_ONE;
   bd.DestBlendAlpha = D3D10_BLEND_ZERO;
   bd.BlendOpAlpha = D3D10_BLEND_OP_ADD;  
   bd.RenderTargetWriteMask[0] = D3D10_COLOR_WRITE_ENABLE_ALL;

   md3dDevice->CreateBlendState(&bd, &pBlendStateAdditive);



   //DepthStencil state

   //Disabled depth writes and enabled depth test
   D3D10_DEPTH_STENCIL_DESC dsd;// = {0};
   ZeroMemory(&dsd, sizeof(D3D10_DEPTH_STENCIL_DESC));

   dsd.DepthEnable = true;
   dsd.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ZERO;
   dsd.DepthFunc = D3D10_COMPARISON_LESS;
   dsd.StencilEnable = false;
   //dsd.StencilReadMask = D3D10_DEFAULT_STENCIL_READ_MASK;
   //dsd.StencilWriteMask = D3D10_DEFAULT_STENCIL_WRITE_MASK;
   dsd.StencilReadMask = 0xFF;
   dsd.StencilWriteMask = 0xFF;

    dsd.FrontFace.StencilFailOp      = D3D10_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilPassOp      = D3D10_STENCIL_OP_REPLACE;
	dsd.FrontFace.StencilFunc        = D3D10_COMPARISON_ALWAYS;

	// We are not rendering backfacing polygons, so these settings do not matter.
    dsd.BackFace.StencilFailOp       = D3D10_STENCIL_OP_KEEP;
	dsd.BackFace.StencilDepthFailOp  = D3D10_STENCIL_OP_KEEP;
	dsd.BackFace.StencilPassOp       = D3D10_STENCIL_OP_REPLACE;
	dsd.BackFace.StencilFunc         = D3D10_COMPARISON_ALWAYS;


   //md3dDevice->CreateDepthStencilState(&dsd, ppDepthStencilState);
		

}

void LightingApp::onResize()
{
	D3DApp::onResize();	

	float aspect = (float)mClientWidth/mClientHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, 0.25f*PI, aspect, 1.0f, 1000.0f);
}

void LightingApp::updateScene(float dt)
{
	D3DApp::updateScene(dt);

	// Every quarter second, generate a random wave.
	static float t_base = 0.0f;
	if( (mTimer.getGameTime() - t_base) >= 0.25f )
	{
		t_base += 0.25f;
 
		DWORD i = 5 + rand() % 250;//400;
		DWORD j = 5 + rand() % 250;//400;

		float r = RandF(1.0f, 2.0f);

		mWaves.disturb(i, j, r);
	}
	

	// Update angles based on input to orbit camera around scene.
	if(GetAsyncKeyState('A') & 0x8000)	mTheta -= 2.0f*dt;
	if(GetAsyncKeyState('D') & 0x8000)	mTheta += 2.0f*dt;
	if(GetAsyncKeyState('W') & 0x8000)	mPhi -= 2.0f*dt;
	if(GetAsyncKeyState('S') & 0x8000)	mPhi += 2.0f*dt;
	if(GetAsyncKeyState('Z') & 0x8000)	mRadius -= 155.0f*dt;
	if(GetAsyncKeyState('X') & 0x8000)	mRadius += 155.0f*dt;
	if(GetAsyncKeyState(VK_ESCAPE) & 0x8000) PostQuitMessage(0);


		if(GetAsyncKeyState(VK_UP) & 0x8000)
		{	
			coneAngleExponential-= 15*1e-2;
			mLights[2].spotPow = coneAngleExponential;
		}


		if(GetAsyncKeyState(VK_DOWN) & 0x8000)
		{	
			coneAngleExponential+= 15*1e-2;
			mLights[2].spotPow = coneAngleExponential;
		}



	// Restrict the angle mPhi and radius mRadius.
	if( mPhi < 0.1f )	mPhi = 0.1f;
	if( mPhi > PI-0.1f)	mPhi = PI-0.1f;

	//if( mRadius < 25.0f) mRadius = 25.0f;

	if(GetAsyncKeyState('1') & 0x8000)	mLightType = 0;
	if(GetAsyncKeyState('2') & 0x8000)	mLightType = 1;
	if(GetAsyncKeyState('3') & 0x8000)	mLightType = 2;
	if(GetAsyncKeyState('4') & 0x8000)  mLightType = 3;

	// Convert Spherical to Cartesian coordinates: mPhi measured from +y
	// and mTheta measured counterclockwise from -z.
	mEyePos.x =  mRadius*sinf(mPhi)*sinf(mTheta);
	mEyePos.z = -mRadius*sinf(mPhi)*cosf(mTheta);
	mEyePos.y =  mRadius*cosf(mPhi);

	// Build the view matrix.
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &mEyePos, &target, &up);


	// The point light circles the scene as a function of time, 
	// staying 7 units above the land's or water's surface.
	mLights[1].pos.x = 50.0f*cosf( mTimer.getGameTime() );
	mLights[1].pos.z = 50.0f*sinf( mTimer.getGameTime() );
	//mLights[1].pos.y = Max(mLand.getHeight(mLights[1].pos.x, mLights[1].pos.z), 0.0f) + 7.0f;
	mLights[1].pos.y = 15;


	// The spotlight takes on the camera position and is aimed in the
	// same direction the camera is looking.  In this way, it looks
	// like we are holding a flashlight.
	mLights[2].pos = mEyePos;
	D3DXVec3Normalize(&mLights[2].dir, &(target-mEyePos));

	//D3DXVECTOR3 vT;

	/*static float CurrTime = 0;

	CurrTime += 3*1e-2;

	if(CurrTime-mTimer.getGameTime() >= 0.2)
	{

		vT.y += 0.1*dt;
		vT.x = 0.25*sin(vT.y);
		vT.z = 0;
	
		D3DXMATRIX mTranslation;

		D3DXMatrixTranslation(&mTranslation, vT.x, vT.y, vT.z);
		//D3DXMatrixTranslation(&mTranslation, 0, 0, vT.z);

		mTexMtxWaves *= mTranslation;

	}
	*/
	/*
		vT.y += 0.1*dt;
		vT.x = 0.25*sin(vT.y);
		vT.z = 0;
	*/

	// Animate water texture as a function of time.
	vT.y += 0.1f*dt;
	//mWaterTexOffset.x = 0.25f*sinf(4.0f*mWaterTexOffset.y); 
	vT.x = 0.25f;//*mWaterTexOffset.y; 

	mWaves.update(dt);
}

void LightingApp::drawScene()
{
	D3DApp::drawScene();
	
	
	// Restore default states, input layout and primitive topology 
	// because mFont->DrawText changes them.  Note that we can 
	// restore the default states by passing null.
	md3dDevice->OMSetDepthStencilState(0, 0);
	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	md3dDevice->OMSetBlendState(0, blendFactor, 0xffffffff);

    md3dDevice->IASetInputLayout(mVertexLayout);
    md3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	// Set per frame constants.
	mfxEyePosVar->SetRawValue(&mEyePos, 0, sizeof(D3DXVECTOR3));

	if (3 != mLightType)
	{
			mfxLightVar->SetRawValue(&mLights[mLightType], 0, sizeof(Light));
		mfxLightType->SetInt(mLightType);
	}
	else
	{
	
		mfxLightVar->SetRawValue(&mLights[1], 0, sizeof(Light));
		mfxLightVar2->SetRawValue(&mLights[2], 0, sizeof(Light));
		mfxLightType->SetInt(mLightType);
	
	}

	D3DXMATRIX S, T;
	
	D3DXMatrixScaling(&S, 5, 5, 1);
	//D3DXMatrixTranslation(&T, vT.x, vT.y, 0);
	D3DXMatrixTranslation(&T, 0, 0, 0);
		

		mTexMtxWaves = S*T; // make water texture moving



		static DWORD k = 0;
		static float t_base = 0;

		
		


    D3D10_TECHNIQUE_DESC techDesc;
    mTech->GetDesc( &techDesc );
	 
    for(UINT i = 0; i < techDesc.Passes; ++i)
    {
        //ID3D10EffectPass* pass = mTech->GetPassByIndex(i);
				

		//mTech->GetPassByIndex( i )->Apply(0);
		
				
		//Draw land
		//md3dDevice->RSSetState(Solid);
		md3dDevice->RSSetState(CullNone);
		
		mWVP = mLandWorld*mView*mProj;
		mfxWVPVar->SetMatrix((float*)&mWVP);
		mfxWorldVar->SetMatrix((float*)&mLandWorld);
		mfxTexMtxVar->SetMatrix((float*)&mTexMtxLand);
		//mfxDiffuseMapVar->SetResource(pGrassMap);
		mfxDiffuseMapVar->SetResource(pGrassMap);
		mfxSpecularMapVar->SetResource(pSpecularMap);

		//md3dDevice->OMSetDepthStencilState(*ppDepthStencilState, 0);

		mTech->GetPassByIndex( i )->Apply(0);
		
		mLand.draw();

		
	
		//Draw crate
		//md3dDevice->RSSetState(Solid);

		mWVP = mCrateWorld*mView*mProj;
		mfxWVPVar->SetMatrix((float*)&mWVP);
		mfxWorldVar->SetMatrix((float*)&mCrateWorld);
		mfxTexMtxVar->SetMatrix((float*)&mTexMtxCrate);
		//mfxDiffuseMapVar->SetResource(pCrateMap);
		//mfxDiffuseMapVar->SetResource(ppLightningAnim);
		
		//if (mTimer.getGameTime()-t_base >= 0.3333)
		//{
			
			//t_base+= 0.1;

				if (k<60)
				{
		
					mfxDiffuseMapVar->SetResource(*(ppLightningAnim+k));
					k++;
		
				}
				else {k = 0;}
		//}
			

		

		//mfxDiffuseMapVar->SetResource(*ppLightningAnim);
			//mfxDiffuseMapVar->SetResource(pCrateMap);

		mfxSpecularMapVar->SetResource(pSpecularMap);
		mTech->GetPassByIndex(i)->Apply(0);
		md3dDevice->OMSetBlendState(pBlendStateAdditive, blendFactor, 0xffffffff);
		/////mCrate.draw();

		//md3dDevice->RSSetState(Wireframe);

				cylinder.draw();

		//md3dDevice->OMSetDepthStencilState(0, 0);

		//Draw water
		
		//md3dDevice->RSSetState(Wireframe);
				md3dDevice->RSSetState(Solid);

		mWVP = mWavesWorld*mView*mProj;
		mfxWVPVar->SetMatrix((float*)&mWVP);
		mfxWorldVar->SetMatrix((float*)&mWavesWorld);		
		mfxTexMtxVar->SetMatrix((float*)&mTexMtxWaves);
		mfxDiffuseMapVar->SetResource(pWavesMap);
		mfxSpecularMapVar->SetResource(pSpecularMap);
		mTech->GetPassByIndex( i )->Apply(0);
		md3dDevice->OMSetBlendState(pBlendStateTransparency, blendFactor, 0xffffffff);
		mWaves.draw();

		
		
    }

	// We specify DT_NOCLIP, so we do not care about width/height of the rect.
	RECT R = {5, 5, 0, 0};
	md3dDevice->RSSetState(0);
	mFont->DrawText(0, mFrameStats.c_str(), -1, &R, DT_NOCLIP, BLACK);

	mSwapChain->Present(0, 0);
}

void LightingApp::buildFX()
{
	DWORD shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
 
	ID3D10Blob* compilationErrors = 0;
	HRESULT hr = 0;
	hr = D3DX10CreateEffectFromFile(L"tex.fx", 0, 0, 
		"fx_4_0", shaderFlags, 0, md3dDevice, 0, 0, &mFX, &compilationErrors, 0);
	if(FAILED(hr))
	{
		if( compilationErrors )
		{
			MessageBoxA(0, (char*)compilationErrors->GetBufferPointer(), 0, 0);
			ReleaseCOM(compilationErrors);
		}
		DXTrace(__FILE__, (DWORD)__LINE__, hr, L"D3DX10CreateEffectFromFile", true);
	} 

	mTech = mFX->GetTechniqueByName("TexTech");
	
	mfxWVPVar     = mFX->GetVariableByName("gWVP")->AsMatrix();
	mfxWorldVar   = mFX->GetVariableByName("gWorld")->AsMatrix();
	mfxTexMtxVar  = mFX->GetVariableByName("gTexMtx")->AsMatrix();
	mfxEyePosVar  = mFX->GetVariableByName("gEyePosW");
	mfxLightVar   = mFX->GetVariableByName("gLight");	
	mfxLightVar2  = mFX->GetVariableByName("gLight2");
	mfxLightType  = mFX->GetVariableByName("gLightType")->AsScalar();		

	mfxDiffuseMapVar  = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	mfxSpecularMapVar = mFX->GetVariableByName("gSpecularMap")->AsShaderResource();
}

void LightingApp::buildVertexLayouts()
{
	// Create the vertex input layout.
	/*
	D3D10_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"DIFFUSE",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"SPECULAR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 40, D3D10_INPUT_PER_VERTEX_DATA, 0}
	};*/


	
	
		// Create the vertex input layout.
		D3D10_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D10_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0},
		};

		// Create the input layout
		D3D10_PASS_DESC PassDesc;
		mTech->GetPassByIndex(0)->GetDesc(&PassDesc);
		HR(md3dDevice->CreateInputLayout(vertexDesc, 3, PassDesc.pIAInputSignature,
			PassDesc.IAInputSignatureSize, &mVertexLayout));
	
	
}
