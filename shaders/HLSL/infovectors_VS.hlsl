
#pragma pack_matrix(row_major)





cbuffer cbPerObject       :  register(b0)
{
	float4x4 WVP;
};



struct VS_IN
{
	float4 posL: POSITION;
	float4 col : COLOR; 
};

struct VS_OUT
{
	float4 posH: SV_POSITION;
	float4 col : COLOR;
};



VS_OUT VS(VS_IN vIn)
{
	VS_OUT vOut;

		vOut.posH = mul(vIn.posL, WVP);
		vOut.col = vIn.col;

	return vOut;

}




