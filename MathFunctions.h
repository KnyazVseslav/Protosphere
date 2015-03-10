

#pragma once

#ifndef MATHFUNCTIONS_H
#define MATHFUNCTIONS_H


#include <DirectXMath.h>
#include <vector>

/*


/*
DirectX::XMVECTOR operator-(DirectX::FXMVECTOR v)
{
	return DirectX::XMVectorNegate(v);
}
*/



/*#include <DirectXPackedVector.h>
#include <DirectXColors.h>





XMVECTOR CrossProduct(FXMVECTOR v1, FXMVECTOR v2)
 {
	return XMVectorSet(v1.y*v2.z-v1.z*v2.y, v1.z*v2.x-v1.x*v2.z, v1.x*v2.y-v1.y*v2.x);
 }


 float DotProduct(FXMVECTOR v1, FXMVECTOR v2)
 {
	return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z;
 }


 float ProjectFloatRangeOnTextureRange(const float& fvalue)
 {
 
	 float r= (-0.5*fvalue+0.5*FLT_MIN)/(-0.5*FLT_MAX+0.5*FLT_MIN);

	 return r;

	 //return (-0.5*fvalue+0.5*FLT_MIN)/(-0.5*FLT_MAX+0.5*FLT_MIN); 
 
 }

XMVECTOR normalize(FXMVECTOR v)
{	

	float length = XMVector3Length(&v);

	if (!length || 1 == length) return v;

	XMVECTOR vOut= v;

	return (vOut/length);

}
*/


//DirectX::XMVECTOR XM_CALLCONV Vec3TransformAsPoint(DirectX::FXMVECTOR  v, DirectX::FXMMATRIX&  m)
//DirectX::XMVECTOR Vec3TransformAsPoint(DirectX::FXMMATRIX&  m, DirectX::FXMVECTOR  v);

DirectX::XMVECTOR Vec3TransformAsPoint(const std::vector<float>&  v, const std::vector< std::vector<float> > &  m);
/*
{
 
		int i, j, k;

		DirectX::XMVECTOR r = DirectX::XMVectorZero();



			for(i= 0; i < 3; i++)
			{

				  for(j= 0; j < 3; j++)
				
					  r.m128_f32[i] += v.m128_f32[j] * m.r[j].m128_f32[i];
			  
			  
			  		 r.m128_f32[i]+= m.r[3].m128_f32[i];

			}

			
	 return r;
 
 }



*/


#endif
 
 
 


