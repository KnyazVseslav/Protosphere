
#include "MathFunctions.h"

//DirectX::XMVECTOR XM_CALLCONV Vec3TransformAsPoint(DirectX::FXMVECTOR v, DirectX::FXMMATRIX& m)
/*
//DirectX::XMVECTOR XM_CALLCONV Vec3TransformAsPoint(DirectX::FXMMATRIX&  m, DirectX::FXMVECTOR  v)
{

	int i, j, k;

	DirectX::XMVECTOR r = DirectX::XMVectorZero();



	for (i = 0; i < 3; i++)
	{

		for (j = 0; j < 3; j++)

			r.m128_f32[i] += v.m128_f32[j] * m.r[j].m128_f32[i];


		r.m128_f32[i] += m.r[3].m128_f32[i];

	}


	return r;

}
*/


DirectX::XMVECTOR Vec3TransformAsPoint(const std::vector<float>&  v, const std::vector< std::vector<float> > &  m)
{

	int i, j, k;

	DirectX::XMVECTOR r = DirectX::XMVectorZero();



	for (i = 0; i < 3; i++)
	{

		for (j = 0; j < 3; j++)

			r.m128_f32[i] += v[j] * m[j][i];


		r.m128_f32[i] += m[3][i];

	}


	return r;

}


