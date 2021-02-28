#include "Common.hlsl"
Texture2D textureColorMap : register(t0,space1);
SamplerState samplerColorMap : register(s0,space1);
Texture2D textureNormalMap : register(t1,space1);
SamplerState samplerNormalMap : register(s1,space1);
Texture2D texturePbrMap : register(t2,space1);
SamplerState samplerPbrMap : register(s2,space1);

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float2 UV: TEXCOORD0;
    float4 WorldPos : TEXCOORD01;
};






struct BxdfData
{
  float3 Albedo;
  float Metallic;
  float Roughness;
  float3 N;
  float3 L;
  float3 V;
  float3 H;
  float NoL;
  float NoH;
  float NoV;
  float VoH;
  float VoL;
};

// Spec GGX
// FGD/(4 nl nv)
float D_GGX( float Roughness, float NoH )
{
	float a = Roughness * Roughness;
	float a2 = a * a;
	float d = ( NoH * a2 - NoH ) * NoH + 1;	// 2 mad
	return a2 / ( PI*d*d );					// 4 mul, 1 rcp
}

float Vis_SmithJointApprox( float a, float NoV, float NoL )
{
	float Vis_SmithV = NoL * ( NoV * ( 1 - a ) + a );
	float Vis_SmithL = NoV * ( NoL * ( 1 - a ) + a );
	return 0.5 * rcp( Vis_SmithV + Vis_SmithL );
}

float Vis_Schlick( float Roughness, float NoV, float NoL )
{
	float k = Roughness * Roughness * 0.5;
	float Vis_SchlickV = NoV * (1 - k) + k;
	float Vis_SchlickL = NoL * (1 - k) + k;
	return 0.25 / ( Vis_SchlickV * Vis_SchlickL );
}

float DielectricSpecularToF0(float Specular)
{
	return 0.08f * Specular;
}

// [Burley, "Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering"]
float DielectricF0ToIor(float F0)
{
	return 2.0f / (1.0f - sqrt(F0)) - 1.0f;
}

float DielectricIorToF0(float Ior)
{
	const float F0Sqrt = (Ior-1)/(Ior+1);
	const float F0 = F0Sqrt*F0Sqrt;
	return F0;
}

float3 ComputeF0(float Specular, float3 BaseColor, float Metallic)
{
	return lerp(DielectricSpecularToF0(Specular).xxx, BaseColor, Metallic.xxx);
}



float3 F_Schlick( float3 SpecularColor, float VoH )
{
	float Fc = Pow5( 1 - VoH );					// 1 sub, 3 mul
	//return Fc + (1 - Fc) * SpecularColor;		// 1 add, 3 mad

	// Anything less than 2% is physically impossible and is instead considered to be shadowing
	return saturate( 50.0 * SpecularColor.g ) * Fc + (1 - Fc) * SpecularColor;
}

// float3 F_Fresnel( float3 SpecularColor, float VoH )
// {
// 	float3 SpecularColorSqrt = sqrt( clamp( float3(0, 0, 0), float3(0.99, 0.99, 0.99), SpecularColor ) );
// 	float3 n = ( 1 + SpecularColorSqrt ) / ( 1 - SpecularColorSqrt );
// 	float3 g = sqrt( n*n + VoH*VoH - 1 );
// 	return 0.5 * Square( (g - VoH) / (g + VoH) ) * ( 1 + Square( ((g+VoH)*VoH - 1) / ((g-VoH)*VoH + 1) ) );
// }

float3 SPEC_GGX(BxdfData bxdfData)
{
  float spec = 0.04;
  if (bxdfData.Metallic != 0)
  {
    spec = 1;
  }
  float a = bxdfData.Roughness*bxdfData.Roughness;
  return F_Schlick(spec.xxx, bxdfData.VoH)
  * D_GGX(bxdfData.Roughness, bxdfData.NoH)
  * Vis_SmithJointApprox(a, bxdfData.NoV, bxdfData.NoL);
}




void PrepareBxdfData(inout BxdfData bxdfData,float3 albedo, float metallic, float roughness,float3 L, float3 V, float3 N)
{
  bxdfData.Albedo = albedo;
  bxdfData.Metallic = metallic;
  bxdfData.Roughness = roughness;
  bxdfData.N = normalize(N);
  bxdfData.V = normalize(V);
  bxdfData.L = normalize(L);
  bxdfData.H = normalize(L+V);
  bxdfData.NoV = abs(dot(N,V)) + 1e-5;
  bxdfData.NoL = dot(N,L);
  bxdfData.NoH = saturate(dot(N,bxdfData.H));
  bxdfData.VoH = saturate(dot(V,bxdfData.H));
  bxdfData.VoL = saturate(dot(V,bxdfData.L));
  //
  //
  //
  // float InvLenH = rsqrt( 2 + 2 * Context.VoL );
	// NoH = saturate( ( Context.NoL + Context.NoV ) * InvLenH );
	// VoH = saturate( InvLenH + InvLenH * Context.VoL );


}

float4 main(VSOutput input) : SV_TARGET
{
  float3 L = PerframeUbo.lightDir.xyz;
  float3 V = input.WorldPos.xyz - PerframeUbo.cameraPos.xyz;
  float3 N = textureNormalMap.Sample(samplerNormalMap, input.UV).rgb;
  N = float3(N.x,N.z,N.y)-0.5f; // Danger hack find more elegant solution later
  float3 albedo = sqrt(textureColorMap.Sample(samplerColorMap, input.UV).rgb);
  float4 metallicRoughness = texturePbrMap.Sample(samplerPbrMap, input.UV);
  BxdfData bxdfData;
  PrepareBxdfData(bxdfData, albedo, 0, 0.95, L, V, N);

  float3 lightColor = float3(1,0.956,0.839)*3.14;

  float3 diffuse = bxdfData.NoL * albedo / PI;
  float3 spec = SPEC_GGX(bxdfData) * bxdfData.NoL;
  return float4(lightColor * (diffuse+spec), 1.0);
}