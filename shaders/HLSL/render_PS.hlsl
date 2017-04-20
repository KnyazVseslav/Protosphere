#include "lighthelper.hlsl"

#pragma pack_matrix(row_major)


static const float DEPTH_EPSILON = 0.001f;
static const float DEPTHMAP_SIZE = 2048.0f;
static const float DEPTHMAP_DX = 1.0f / DEPTHMAP_SIZE;



cbuffer cbPerFrame     :  register(b0)
{	
	float4x4 WV;
	
	float4 LightDir;
	float4 LightPos;
	float4 pos_test2;

	float4 L_ambient;
	float4 L_diffuse;
	float4 L_specular;

	int LightSourceType; 
	LightSource light;
	LightSource light2;
	float4 EyePosW;

};


//LightSource light;
//	LightSource light2;



Texture2D depthMap     :  register(t0); // for shadow mapping - gets assigned with depth map SRV

Texture2D DiffuseMap   :  register(t1);
Texture2D SpecularMap  :  register(t2);

Texture2D OverlayMap   :  register(t3);



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

// For shadow factor
SamplerComparisonState smCMP
{
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;

	BorderColor = float4(0, 0, 0, 0);

	ComparisonFunc = LESS_EQUAL;
	//ComparisonFunc = GREATER_EQUAL;
	//ComparisonFunc = GREATER;


};


struct PS_IN
{		
	float4 posH        : SV_POSITION;
	float4 posWV       : POSITION;
	float4 normalWV    : NORMAL;
	float4 vel         : VELOCITY;
	float4 texC        : TEXCOORD;	

	float4 PointInLS_H : TEXCOORD1; // for shadow mapping
};



float CalcShadowFactor_CmpLevelZero(float4 PointInLS_H) //_CmpLevelZero
{

	// Complete transform to LS NDC space by doing division by w
	// Reason: 1) depthMap values (depthes) are also in NDC; 2) more convenient to transform to texture space
	// Also works for orthographically projected points, since in that case w = 1
	PointInLS_H.xyz /= PointInLS_H.w;

	// Depth in NDC space
	float depth = PointInLS_H.z;


	//Depth map texel size
	float dx = DEPTHMAP_DX;

	float percentLit = 0.0f;

	// ѕриращени€ дл€ проективных выборок из depth map по квадрату вокруг преобразованной в ls_VP*NDC*TexSpace точки сцены (u, v)

	/*
	
		*----*----*
		|         |
		*    *    *
		| *(u, v) |
		*----*----*

	*/

	[unroll]
	const float2 offsets[9] =
	{
		float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
	};



	// ¬ыборки и интерпол€ци€ между ними, накопление результатов

	for (uint i = 0; i < 9; ++i)
	{
														 // point where to sample  // comparison value
		percentLit += depthMap.SampleCmpLevelZero(smCMP, PointInLS_H.xy + offsets[i], depth);
	}

	// return average value
	return percentLit /= 9.0f;


}


float CalcShadowFactor(float4 PointInLS_H)
{
	// Complete transform to LS NDC space by doing division by w
	// Reason: 1) depthMap values (depthes) are also in NDC; 2) more convenient to transform to texture space
	// Also works for orthographically projected points, since in that case w = 1
	PointInLS_H.xyz /= PointInLS_H.w;

	// In NDC space points' coordinates have ranges: x,y: [-1, 1] ;  z: [0, 1]
	// We need to clip points outside the light volume since they are in shadow, i.e. their 
	if( PointInLS_H.x < -1.0f || PointInLS_H.x > 1.0f || 
	    PointInLS_H.y < -1.0f || PointInLS_H.y > 1.0f ||
	    PointInLS_H.z < 0.0f )
	    return 0.0f;
	  
	// Transform from light source projection NDC (x = -1..1, y = -1..1) space
	// to texture space (u = 0..1, v = 0..1) of a depthMap
	PointInLS_H.x = +0.5f*PointInLS_H.x + 0.5f;
	PointInLS_H.y = -0.5f*PointInLS_H.y + 0.5f;
	
	// Depth in NDC space (0..1)
	float depth = PointInLS_H.z;

	// Sample shadow map to get nearest depth to scene point (uv)
	float s0 = depthMap.Sample(sm_Shadow, PointInLS_H.xy).r;
	float s1 = depthMap.Sample(sm_Shadow, PointInLS_H.xy + float2(DEPTHMAP_DX, 0)).r;
	float s2 = depthMap.Sample(sm_Shadow, PointInLS_H.xy + float2(0, DEPTHMAP_DX)).r;
	float s3 = depthMap.Sample(sm_Shadow, PointInLS_H.xy + float2(DEPTHMAP_DX, DEPTHMAP_DX)).r;
	
	// Shadow map тест: находитс€ ли пиксель в тени если сравнивать его глубину с четырьм€ ближайшими выборками из depthmap?

		/*
	
		----s0*----------s1*-----
			  |			   |
			  |	 *(u, v)   |
			  |		depth  |
			  |			   |
		----s2*----------s3*-----
	
		*/

	float result0 = depth <= s0 + DEPTH_EPSILON;
	float result1 = depth <= s1 + DEPTH_EPSILON;
	float result2 = depth <= s2 + DEPTH_EPSILON;
	float result3 = depth <= s3 + DEPTH_EPSILON;	



	// ѕреобразование точки сцены, преобразованной в текстурное пространство (u = 0..1, v = 0..1), 
	// к пространству (значений) depthmap (DEPTHMAP_SIZE X DEPTHMAP_SIZE)
	float2 texelPos = DEPTHMAP_SIZE*PointInLS_H.xy;
	
	// Determine the interpolation amounts
	float2 t = frac( texelPos );
	
	

// »нтерпол€ци€ результатов shadow map теста (билинейна€ интерпол€ци€ - линейна€ интерпол€ци€ линейных интерпол€ций)
 return lerp( lerp(result0, result1, t.x), lerp(result2, result3, t.x), // interpolate along x-axis first between s0 and s1, then between s2 and s3
	               t.y ); // and then - along y-axis between two previous lerps

	/*

	 -----*----s0s1*---*-----
	      |		   |   |
		  |	*(u,v) |   |
		  |		   * lerp(s0s1, s2s3, t.y) 
		  |		   |   |
	------*----s2s3*---*-----

	*/
	
				  
}

bool Equal(float4x4 m1, float4x4 m2)
{

	for(dword i= 0; i < 4; i++)
	 for(dword j= 0; j < 4; j++)
	   if(m1[i][j] != m2[i][j]) return false;

	return true;

}


float4 PS(PS_IN pIn): SV_TARGET
{

	//Get materials from texture maps
	float4 diffuse  = DiffuseMap.Sample(sm_Trilinear, pIn.texC);
	float4 specular = diffuse;
	//float4 specular = SpecularMap.Sample(sm_Trilinear, pIn.texC);
	//float4 specular = float4(0, 0, 1, 1);


	////float4 overlay = OverlayMap.Sample(sm_Trilinear, pIn.texC);


	//float4 multitex = mul(diffuse, overlay);
	////float4 multitex = diffuse*overlay;


	//Map [0, 1] --> [0, 256]
	specular.a *= 256.0f;

	//Interpolating normal can make it not be of unit length
	float4 n = normalize(pIn.normalWV);

	//Compute the lit color for this pixel
	SurfaceInfo v = {pIn.posWV, n, diffuse, specular};
	//SurfaceInfo v = {pIn.posW, n, multitex, specular};

	
	float shadow_factor = CalcShadowFactor(pIn.PointInLS_H);
	//float shadow_factor = 1.0f;

	
	float4 litColor;


	if (1 == LightSourceType)
		litColor = ParallelLight(v, light, LightDir, EyePosW, WV, shadow_factor);		 
		//litColor = ParallelLight(v, light, LightDir, EyePosW, WV);
	
	if (2 == LightSourceType)
		 litColor = PointLight(v, light, LightPos, pos_test2, EyePosW, WV, shadow_factor, L_ambient, L_diffuse, L_specular);

	if (3 == LightSourceType)
		// litColor = Spotlight(v, light, EyePosW, WV);
			 litColor = Spotlight(v, light, LightDir, LightPos, pos_test2, EyePosW, WV, shadow_factor, L_ambient, L_diffuse, L_specular);

	


	//if(0==pIn.normalWV.x && 0==pIn.normalWV.y && 0==pIn.normalWV.z) return float4(0, 0, 1, 0);

	//if(Equal(light.basis, LightWVP)) return float4(1,0,0,0);

	//return float4(shadowFactor, shadowFactor, shadowFactor, 0);

	//if(depthMap == ) return float4(shadowFactor, shadowFactor, shadowFactor, 0);
	
	
	//return shadowFactor;
		

	litColor.a= diffuse.a;

    //return float4(pIn.PointInLS_H.xyz / pIn.PointInLS_H.w, 1.f);
    //return depthMap.Sample(sm_Trilinear, pIn.texC.xy);
    return litColor;
    //return pIn.normalWV;

					


/*
	float4 r= float4(1, 1, 1, 1);
	if(0 == n.x && 0 == n.y && 0 == n.z) r = float4(0,0,1,1);
*/

	
	//float4 r= float4(n.x, n.y, n.z, 1);
	//float4 r= float4(WV[3][0], WV[3][1], WV[3][2], WV[3][3]);
	//float4 r= float4(WV[1][0], WV[1][1], WV[1][2], WV[1][3]);

	//return r;
	

}




