cbuffer cbPerObject
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

float4 PS(VS_OUT pIn): SV_TARGET
{
	return pIn.col;
}


technique11 InfoTech
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
			SetGeometryShader(NULL);
		SetPixelShader( CompileShader( ps_5_0, PS() ));
	}
}