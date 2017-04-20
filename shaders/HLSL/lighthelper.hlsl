struct LightSource
{
	float4 pos;
	float4 right;
	float4 up;

	float4 dir;		
	float4 ambient;
	float4 diffuse;
	float4 specular;
	
	float4 att;

	float  spotPower;
	float  range;
	float theta; // Max cone angle
	int LSType;
};

struct SurfaceInfo
{
	float4 pos;
    float4 normal;
    float4 diffuse;
    float4 specular;
};


float4 ParallelLight(SurfaceInfo v, LightSource L, float4 Ldir, float4 eyePos, float4x4 m, float shadowFactor)
{
	
	float4 litColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
 
	
	float3 dir = normalize(Ldir);
			dir = mul(float4(dir, 0), m);
	float3 lightVec = normalize(-dir);

	v.normal = normalize(v.normal);
	
	float diffuseFactor = dot(lightVec, v.normal);

	float specularFactor = 0;

		if(diffuseFactor > 0) // отбраковка параллельных и невидимых поверхностей
		{
			float4 observer_pos_tmp = mul(eyePos, m);
				float3 observer_pos;  observer_pos.xyz = observer_pos_tmp.xyz;

			float3 surface_to_observer = normalize(eyePos - v.pos);
			float3 dir_reflected = normalize(reflect(dir, v.normal));
			
			specularFactor = pow(max(dot(surface_to_observer, dir_reflected), 0), 128);
		}

		if(0 == specularFactor) return litColor = float4(0, 0, 0, 1);

		return litColor =  diffuseFactor * v.diffuse * L.diffuse + specularFactor * v.specular * L.specular;

}



float4 PointLight(SurfaceInfo v, LightSource L, float4 LightPos, float4 test_v, float4 eyePos, float4x4 m, float shadowFactor,
				  float4 L_ambient, float4 L_diffuse, float4 L_specular)
{

	float4 litColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

      float4 light_pos4= LightPos;

		light_pos4.w= 1;

		float4 Lpos4 = mul(light_pos4, m);

		float3 Lpos3;

	      Lpos3.xyz = Lpos4.xyz;

	
		  float3 lightVec = Lpos3 - v.pos;
	
		
	// The distance from surface to light.
	float d = length(lightVec);
	
		
	// Нормализовать световой вектор
	lightVec /= d; 
	
	// Фоновая составляющая
	litColor += v.diffuse * L_ambient;	
	//litColor += float4(1, 1, 1, 1) * L_ambient;
	
	// Если поверхность находится в поле действия источника света
	float diffuseFactor = dot(lightVec, v.normal);

	[branch]
	if( diffuseFactor > 0.0f )
	{
		float specPower  = max(v.specular.a, 1.0f);
		float3 toEye     = normalize(eyePos - v.pos);
		float3 R         = reflect(-lightVec, v.normal);
		float specFactor = pow(max(dot(R, toEye), 0.0f), 128);
		litColor += shadowFactor*diffuseFactor * v.diffuse * L_diffuse;
		litColor += shadowFactor*specFactor * v.specular * L_specular;
	}

	return litColor;
}



float4 Spotlight(SurfaceInfo v, LightSource L, float4 LightDir, float4 LightPos, 
				   float4 test_v, float4 eyePos, float4x4 m, float shadowFactor,
				     float4 L_ambient, float4 L_diffuse, float4 L_specular)
{

	float4 litColor_pl = PointLight(v, L, LightPos, test_v, eyePos, m, shadowFactor, L_ambient, L_diffuse, L_specular);
		float4 litColor = litColor_pl;
		return litColor;

	/*
      float4 light_pos4= LightPos;

		light_pos4.w= 1;

		float4 Lpos4 = mul(light_pos4, m);

		float3 Lpos3;

	      Lpos3.xyz = Lpos4.xyz;
	
		  // Вектор от точки поверхности к ИС
		  float3 lightVec = normalize(Lpos3 - v.pos).xyz;
	
	
	float3 Ldir = normalize(LightDir).xyz;
			Ldir = normalize(mul(float4(Ldir, 0), m));

	float cosineLightVertex= dot(-lightVec, Ldir); // косинус угла между направлением ИС и вектором от ИС к точке поверхности

			
			float s = 0;

if(dot(v.normal, Ldir) < 0)	
	 s = pow(max(cosineLightVertex, 0), 8);

	
	return litColor*s;
	*/

}


 
 