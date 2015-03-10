


Texture2D ObjectTexture  :  register(t0); 



SamplerState samTrilinearAddrUVWrap 
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};




struct PS_IN
{
	float4 PosH : SV_POSITION;
	float4 TexC : TEXCOORD;
};



void PS(PS_IN v)
{

	


	/*	
		// !! This shader is needed when there are transparent materials in the  scene, which we don't need to cast a shadow !!
	
		// Uncomment when all objects will have their textures being bound
		// with global ObjectTexture, otherwise all scene pixels will be
		// clipped out and won't get written to the depth map since
		// for the empty texture expression diffuse.a won- 0.15f will always
		// result in value less than zero.



		float4 diffuse = ObjectTexture.Sample(samTrilinearAddrUVWrap, v.TexC);


			// Shadows can be cast only by visible pixels;
			// clip(arg) discards the current pixel from further processing if arg < 0 , 
			// therefore if pixel's alpha < 0.15 then the pixel will be discarded
			clip(diffuse.a - 0.15f);
	*/

}