
Texture2D DiffuseMap   :  register(t0); // ��� �������� ����� ������ ����� SRV

// ������������ ��������
Texture2D SpecularMap  :  register(t1);
Texture2D OverlayMap   :  register(t2);


SamplerState sm_Trilinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV= WRAP;
};



struct PS_IN
{
	float4 posH: SV_POSITION;
	float3 norm: NORMAL;
	float3 vel: VELOCITY;
	float2 texC: TEXCOORD;
};




float4 PS(PS_IN v): SV_TARGET
{
	float4 OutputColor;

			OutputColor= DiffuseMap.Sample(sm_Trilinear, v.texC); // �������� ��� ������� - ������� ������� �� �������� � ������ ����� �� �����			

	return OutputColor;
			//return float4(1, 0, 0, 1);
}


