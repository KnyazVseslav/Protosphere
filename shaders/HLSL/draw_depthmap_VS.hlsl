// ������ ��� ��������� ����� ������; ������ ��� �������� ����, ��� ��������� ��������; ������ ��������� �� ���������
// �������� ��� ��������� � ���������� �������


#pragma pack_matrix(row_major)


cbuffer cbPerObject    :  register(b0)
{
	float4x4 LWVP;
};


struct VS_IN
{
	float3 posL: POSITION;
	float3 norm: NORMAL;
	float3 vel: VELOCITY;
	float2 texC: TEXCOORD;
};


struct VS_OUT
{
	float4 posH: SV_POSITION;
	float3 norm: NORMAL;
	float3 vel: VELOCITY;
	float2 texC: TEXCOORD;
};



// Pass-through (��������) ������
VS_OUT VS(VS_IN vIn)
{
	VS_OUT vOut;

		vOut.posH = mul(float4(vIn.posL, 1.0f), LWVP); // �������������� ������������ ����� ��������� ��� ���������� ���������� � �������������
													   // ���������� ������ ������� ����� ���������� ������� � ���������� ������������ ���������
		vOut.norm = vIn.norm;
		vOut.vel = vIn.vel;
		vOut.texC = vIn.texC;

	return vOut;
}








