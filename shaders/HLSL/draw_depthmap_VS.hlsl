// Шейдер для отрисовки карты глубин; просто для проверки того, как заполнена текстура; обычно выводится на плоскость
// Основной код находится в пиксельном шейдере


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



// Pass-through (сквозной) шейдер
VS_OUT VS(VS_IN vIn)
{
	VS_OUT vOut;

		vOut.posH = mul(float4(vIn.posL, 1.0f), LWVP); // преобразование производится чисто формально для выполнения соглашения о необходимости
													   // нахождения вершин объекта после вершинного шейдера в однородном пространстве отсечения
		vOut.norm = vIn.norm;
		vOut.vel = vIn.vel;
		vOut.texC = vIn.texC;

	return vOut;
}








