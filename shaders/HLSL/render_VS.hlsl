
#pragma pack_matrix(row_major)





cbuffer cbPerScene         :  register(b0)
{
	float4x4 WV;
	float4x4 View;

	float4x4 LightWVP; // for shadow mapping
	
};


cbuffer cbPerObject       :  register(b1)
{
	float4x4 World;
	//float4x4 LWVP;
	float4x4 LWVP;

	float4x4 TexMtx;

	//float4x4 LightWVP; // for shadow mapping
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
	// ������ ������� ��������������
	vOut.posWV = mul(vIn.posL, WV);
	vOut.normalWV = mul(vIn.normalL, WV);
	

	 //Transform to camera's homogeneous clip space
	// ������� � ������������ ��������������
	 //vOut.posH = mul(vIn.posL, transpose(LWVP));
	vOut.posH = mul(vIn.posL, LWVP);
	//vOut.posH = mul(LWVP, vIn.posL);


	 // Texture transformations (optional)
	 vOut.texC = mul(vIn.texC, TexMtx);
	
	 // Transform to light source homogeneous clip space, no NDC transform yet
	 // ������ �������� ����� � �� �� � ������� �� ������� �������� (��� P ����� ���� �������� �������������, ��������������� � ��. ��������),
	 // �������� ������� ����� � ������������� ����� �� ��� ������������ ����
	 vOut.PointInLS_H = mul(vIn.posL, LightWVP); // ����� ����� ���������� w ������ ����� ����� ����� �������� (�� ��������� ������������ ��������)
												 // ���������� z ���� �� ����� - ����� ��� ����������� ��� �������������� ������� ��� ������������
												 // ��������� ����� (��� �������� NDC-������������), �.�. ���������� ����� � ���������� ��������:
												 // -1 <= x' <= 1 ; -1 <= y' <= 1 ; 0 <= z' <= 1


	/*

	//Transform to world-view space	(for lighting calculations)
	// ������ ������� ��������������

	VS_IN v = vIn;

	//v.posL.w= 1.0f; 
	//v.normalL.w= 0.0f; 
	v.texC.zw= 0.0f;

	vOut.posWV = mul(v.posL, WV);
	vOut.normalWV = mul(v.normalL, WV);
	

	 //Transform to camera's homogeneous clip space
	// ������� � ������������ ��������������
	 vOut.posH = mul(v.posL, LWVP);

	 // Texture transformations (optional)
	 vOut.texC = mul(v.texC, TexMtx);
	
	 //Transform to light's homogeneous clip space
	 vOut.PointInLS_H = mul(v.posL, LightWVP);
	 */

	 return vOut;

}


