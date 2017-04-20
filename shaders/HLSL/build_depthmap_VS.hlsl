
#pragma pack_matrix(row_major)


cbuffer cbPerFrame       :  register(b0)
{
	float4x4 LightWVP;
};


struct VS_IN
{
	float4 lPos:  POSITION;
	float4 lNorm: NORMAL;
	float4 vel:   VELOCITY;
	float4 TexC:  TEXCOORD;
};


struct VS_OUT
{
	float4 PosH : SV_POSITION;
	float4 TexC : TEXCOORD;
};


VS_OUT VS(VS_IN vIn)
{
	VS_OUT vOut;
    vOut.PosH = mul(vIn.lPos, LightWVP); // перевод точек сцены к видовому и проекционному пространствам источника света	
	vOut.TexC = vIn.TexC; // texture coordinates left unchanged
	return vOut;
}


