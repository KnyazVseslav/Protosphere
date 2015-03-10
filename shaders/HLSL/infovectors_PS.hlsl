//Шейдер для отрисовки карты глубин; просто для проверки того как заполнена текстура; обычно выводится на плоскость

struct PS_IN
{
	float4 posH: SV_POSITION;
	float4 col : COLOR;
};




float4 PS(PS_IN pIn): SV_TARGET
{
	return pIn.col;
}


