#pragma once

#ifndef Vertex_H
#define Vertex_H

#include <DirectXMath.h>


struct VertexPosNormTex
{
	VertexPosNormTex()
	{
		pos= DirectX::XMVectorZero();
		normal= DirectX::XMVectorZero(); 		
		texC= DirectX::XMVectorZero();
	}

	VertexPosNormTex(float x, float y, float z, 
					 float nx, float ny, float nz, 
					 float u, float v)
	{
	
		pos= DirectX::XMVectorSet(x, y, z, 0);
		normal= DirectX::XMVectorSet(nx, ny, nz, 0);
		texC= DirectX::XMVectorSet(u, v, 0, 0);
	
	}


	DirectX::XMVECTOR pos;
	DirectX::XMVECTOR normal;
	DirectX::XMVECTOR texC;

};


struct VertexPosNormVelocityTex
{

	VertexPosNormVelocityTex() 
	{
		pos= DirectX::XMVectorZero();
		normal= DirectX::XMVectorZero(); 
		vel= DirectX::XMVectorZero();
		texC= DirectX::XMVectorZero();
	}

	VertexPosNormVelocityTex(float x, float y, float z, 
							 float nx, float ny, float nz, 
							 float vx, float vy, float vz, 
							 float u, float v) 
	{
		pos= DirectX::XMVectorSet(x, y, z, 0);
		normal= DirectX::XMVectorSet(nx, ny, nz, 0); 
		vel= DirectX::XMVectorSet(vx, vy, vz, 0);
		texC= DirectX::XMVectorSet(u, v, 0, 0);
	}
/*
	VertexPosNormVelocityTex(const VertexPosNormVelocityTex& v)
	{
	
		if(this != &v)
		{
		

			
		}
	
	}

*/


	DirectX::XMVECTOR pos;
	DirectX::XMVECTOR normal;
	DirectX::XMVECTOR vel;
	DirectX::XMVECTOR texC;


};


struct VertexPosColor
{
	DirectX::XMVECTOR pos;
	DirectX::XMVECTOR color;


	VertexPosColor()
	{
		pos= DirectX::XMVectorZero();
		color= DirectX::XMVectorZero();
	}

	VertexPosColor(float x, float y, float z, 
			       float r, float g, float b, float a)
	{
	
		pos= DirectX::XMVectorSet(x, y, z, 0);
		color= DirectX::XMVectorSet(r, g, b, a);
	
	}

	VertexPosColor(DirectX::XMVECTOR vPos, DirectX::XMVECTOR col)
	{
		pos= vPos;
		color= col;
	}
		

};

#endif

