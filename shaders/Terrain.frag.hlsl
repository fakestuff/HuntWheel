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

struct UBO
{
	row_major float4x4 view;
	row_major float4x4 proj;
  row_major float4x4 invVPF;
  float4 cameraPos;
};
cbuffer ubo : register(b0) { UBO ubo; }


#define PI 3.1415926

struct BxdfData
{
  float3 Albedo;
  float3 Spec;
  float Roughness;
  float3 N;
  float3 L;
  float3 V;
  float3 H;
  float NoL;
  float NoH;
  float NoV;
  float LoH;
  float VoH;
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

float Vis_Schlick( float Roughness, float NoV, float NoL )
{
	float k = Roughness * Roughness * 0.5;
	float Vis_SchlickV = NoV * (1 - k) + k;
	float Vis_SchlickL = NoL * (1 - k) + k;
	return 0.25 / ( Vis_SchlickV * Vis_SchlickL );
}

float Pow2(float x)
{
  return x*x;
}

float Pow4(float x)
{
  float x2 = Pow2(x);
  return x2 * x2;
}

float Pow5(float x)
{
  return Pow4(x) * x;
}

float3 F_Schlick( float3 SpecularColor, float VoH )
{
	float Fc = Pow5( 1 - VoH );					// 1 sub, 3 mul
	//return Fc + (1 - Fc) * SpecularColor;		// 1 add, 3 mad

	// Anything less than 2% is physically impossible and is instead considered to be shadowing
	return saturate( 50.0 * SpecularColor.g ) * Fc + (1 - Fc) * SpecularColor;
}

float3 PBR_SPEC(BxdfData bxdfData)
{
    return F_Schlick(bxdfData.Spec, bxdfData.VoH) 
    * D_GGX(bxdfData.Roughness, bxdfData.NoH) 
    * Vis_Schlick(bxdfData.Roughness, bxdfData.NoV, bxdfData.NoL) 
    / (4 * bxdfData.NoL * bxdfData.NoV);
}

BxdfData PrepareBxdfData(float3 albedo, float3 spec, float roughness,float3 L, float3 V, float3 N)
{
  BxdfData bxdfData;
  bxdfData.Albedo = albedo;
  bxdfData.Spec = spec;
  bxdfData.Roughness = roughness;
  bxdfData.N = N;
  bxdfData.V = V;
  bxdfData.L = L;
  bxdfData.H = normalize(L+V);
  bxdfData.NoV = dot(N,V);
  bxdfData.NoL = dot(N,L);
  bxdfData.NoH = dot(N,bxdfData.H);
  bxdfData.LoH = dot(L,bxdfData.H);
  bxdfData.VoH = dot(V,bxdfData.H);

  return bxdfData;
}

float4 main(VSOutput input) : SV_TARGET
{
  float3 L = normalize(float3(0,1,0));
  float3 V = normalize(input.WorldPos.xyz - ubo.cameraPos.xyz);
  float3 N = normalize(textureNormalMap.Sample(samplerNormalMap, input.UV).rbg*2-1);
  float3 albedo = textureColorMap.Sample(samplerColorMap, input.UV).rgb;
  float4 metallicRoughness = texturePbrMap.Sample(samplerPbrMap, input.UV);
  BxdfData bxdfData = PrepareBxdfData(albedo,albedo,0.95, L, V, N);
  

  //return float4(1,1,1,1);
  float3 lightColor = float3(1,0.956,0.839);
  

  
  
  float3 diffuse = bxdfData.NoL * albedo;
  float3 spec = PBR_SPEC(bxdfData);

  return float4(lightColor * (diffuse), 1.0);
}