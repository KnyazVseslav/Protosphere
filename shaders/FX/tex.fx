#include "lighthelper.fx"

static const float DEPTH_EPSILON = 0.001f;
static const float DEPTHMAP_SIZE = 1024.0f;
static const float DEPTHMAP_DX = 1.0f / DEPTHMAP_SIZE;


cbuffer cbPerFrame
{	
	float4 LightDir;
	float4 LightPos;
	float4 pos_test2;

	float4 L_ambient;
	float4 L_diffuse;
	float4 L_specular;

};

cbuffer cbPerScene
{
	float4x4 WV;
	float4x4 View;

	float4x4 LightWVP; // for shadow mapping
	LightSource light;
	LightSource light2;
	int LightSourceType; 	
	float3 EyePosW;
};


cbuffer cbPerObject
{
	float4x4 World;
	float4x4 LWVP;

	float4x4 TexMtx;

	//float4x4 LightWVP; // for shadow mapping
};


//LightSource light;
//	LightSource light2;

Texture2D depthMap; // for shadow mapping

Texture2D DiffuseMap;
Texture2D SpecularMap;

Texture2D OverlayMap;



SamplerState sm_Trilinear
{

	Filter = MIN_MAG_MIP_LINEAR;

	AddressU = WRAP;
	AddressV = WRAP;	

};


// for shadow mapping
SamplerState sm_Shadow
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};


struct VS_IN
{

	float4 posL    : POSITION;
	float4 normalL : NORMAL;
	float4 vel     : VELOCITY;
	float4 texC    : TEXCOORD;

};

struct VS_OUT
{
		
	float4 posH        : SV_POSITION;
	float4 posWV       : POSITION;
	float4 normalWV    : NORMAL;
	float4 vel         : VELOCITY;
	float4 texC        : TEXCOORD;	

	float4 PointInLS_H : TEXCOORD1; // for shadow mapping

};

VS_OUT VS(VS_IN vIn)
{

	VS_OUT vOut;
	
	
	//Transform to world-view space	(for lighting calculations)
	// Только видовое преобразование
	vOut.posWV = mul(vIn.posL, WV);
	vOut.normalWV = mul(vIn.normalL, WV);
	

	 //Transform to camera's homogeneous clip space
	// Видовое и проекционное преобразования
	 vOut.posH = mul(vIn.posL, LWVP);

	 // Texture transformations (optional)
	 vOut.texC = mul(vIn.texC, TexMtx);
	
	 //Transform to light's homogeneous clip space
	 vOut.PointInLS_H = mul(vIn.posL, LightWVP);


	/*

	//Transform to world-view space	(for lighting calculations)
	// Только видовое преобразование

	VS_IN v = vIn;

	//v.posL.w= 1.0f; 
	//v.normalL.w= 0.0f; 
	v.texC.zw= 0.0f;

	vOut.posWV = mul(v.posL, WV);
	vOut.normalWV = mul(v.normalL, WV);
	

	 //Transform to camera's homogeneous clip space
	// Видовое и проекционное преобразования
	 vOut.posH = mul(v.posL, LWVP);

	 // Texture transformations (optional)
	 vOut.texC = mul(v.texC, TexMtx);
	
	 //Transform to light's homogeneous clip space
	 vOut.PointInLS_H = mul(v.posL, LightWVP);
	 */

	 return vOut;

}


float CalcShadowFactor(float4 PointInLS_H)
{
	// Complete transform to LS NDC space by doing division by w
	PointInLS_H.xyz /= PointInLS_H.w;
/*	
	// In case of using orthographic volume - points outside the light volume are in shadow
	if( PointInLS_H.x < -1.0f || PointInLS_H.x > 1.0f || 
	    PointInLS_H.y < -1.0f || PointInLS_H.y > 1.0f ||
	    PointInLS_H.z < 0.0f )
	    return 0.0f;
	*/    
	// Transform from light source NDC (x = -1..1, y = -1..1) space to texture space (u = 0..1, v = 0..1) of a depthMap
	PointInLS_H.x = +0.5f*PointInLS_H.x + 0.5f;
	PointInLS_H.y = -0.5f*PointInLS_H.y + 0.5f;
	
	// Depth in NDC space
	float depth = PointInLS_H.z;

	
	// Sample shadow map to get nearest depth to scene point (uv)
	float s0 = depthMap.Sample(sm_Shadow, PointInLS_H.xy).r;
	float s1 = depthMap.Sample(sm_Shadow, PointInLS_H.xy + float2(DEPTHMAP_DX, 0)).r;
	float s2 = depthMap.Sample(sm_Shadow, PointInLS_H.xy + float2(0, DEPTHMAP_DX)).r;
	float s3 = depthMap.Sample(sm_Shadow, PointInLS_H.xy + float2(DEPTHMAP_DX, DEPTHMAP_DX)).r;
	
	// Is the pixel depth <= depth map value?
	float result0 = depth <= s0 + DEPTH_EPSILON;
	float result1 = depth <= s1 + DEPTH_EPSILON;
	float result2 = depth <= s2 + DEPTH_EPSILON;
	float result3 = depth <= s3 + DEPTH_EPSILON;	
	
	// Transform to texel space
	float2 texelPos = DEPTHMAP_SIZE*PointInLS_H.xy;
	
	// Determine the interpolation amounts
	float2 t = frac( texelPos );

	/*
	
	----s0*----------s1*-----
	      |			   |
		  |	 *(u, v)   |
		  |		depth  |
		  |			   |
	----s2*----------s3*-----
	
	*/

	//return float4(s0, s1, s2, s3);

// Interpolate results
	return lerp( lerp(result0, result1, t.x), lerp(result2, result3, t.x), // interpolate along x-axis
	               t.y ); // along y-axis between 2 floats (lerp(result0, result1, t.x) and lerp(result2, result3, t.x))
				  
}

bool Equal(float4x4 m1, float4x4 m2)
{

	for(dword i= 0; i < 4; i++)
	 for(dword j= 0; j < 4; j++)
	   if(m1[i][j] != m2[i][j]) return false;

	return true;

}


float4 PS(VS_OUT pIn): SV_TARGET
{

	//Get materials from texture maps
	float4 diffuse  = DiffuseMap.Sample(sm_Trilinear, pIn.texC);
	float4 specular = diffuse;
	//float4 specular = SpecularMap.Sample(sm_Trilinear, pIn.texC);
	//float4 specular = float4(0, 0, 1, 1);


	float4 overlay = OverlayMap.Sample(sm_Trilinear, pIn.texC);


	//float4 multitex = mul(diffuse, overlay);
	float4 multitex = diffuse*overlay;


	//Map [0, 1] --> [0, 256]
	specular.a *= 256.0f;

	//Interpolating normal can make it not be of unit length
	float4 n = normalize(pIn.normalWV);

	//Compute the lit color for this pixel
	SurfaceInfo v = {pIn.posWV, n, diffuse, specular};
	//SurfaceInfo v = {pIn.posW, n, multitex, specular};

	
	float shadowFactor = CalcShadowFactor(pIn.PointInLS_H);

	
	float4 litColor;


	if (1 == LightSourceType)
		litColor = ParallelLight(v, light, LightDir, EyePosW, WV, shadowFactor);		 
		//litColor = ParallelLight(v, light, LightDir, EyePosW, WV);
	
	if (2 == LightSourceType)
		 litColor = PointLight(v, light, LightPos, pos_test2, EyePosW, WV, shadowFactor, L_ambient, L_diffuse, L_specular);

	if (3 == LightSourceType)
		// litColor = Spotlight(v, light, EyePosW, WV);
			 litColor = Spotlight(v, light, LightDir, LightPos, pos_test2, EyePosW, WV, shadowFactor, L_ambient, L_diffuse, L_specular);

	


	//if(0==pIn.normalWV.x && 0==pIn.normalWV.y && 0==pIn.normalWV.z) return float4(0, 0, 1, 0);

	//if(Equal(light.basis, LightWVP)) return float4(1,0,0,0);

	//return float4(shadowFactor, shadowFactor, shadowFactor, 0);

	//if(depthMap == ) return float4(shadowFactor, shadowFactor, shadowFactor, 0);
	
	
	//return shadowFactor;
		

	litColor.a= diffuse.a;

	   
	return litColor;

					


/*
	float4 r= float4(1, 1, 1, 1);
	if(0 == n.x && 0 == n.y && 0 == n.z) r = float4(0,0,1,1);
*/

	/*
	float4 r= float4(n.x, n.y, n.z, 1);

	return r;
	*/

}

technique11 TexTech
{

	pass P0
	{
	
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
			SetGeometryShader(NULL);
		SetPixelShader( CompileShader( ps_5_0,  PS() ) );
	
	}

}