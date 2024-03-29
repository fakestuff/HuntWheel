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

float4 main(VSOutput input) : SV_TARGET
{
  //return float4(1,1,1,1);
  float4 normalMap = textureNormalMap.Sample(samplerNormalMap, input.UV);
  float4 metallicRoughness = texturePbrMap.Sample(samplerPbrMap, input.UV);
  return float4(textureColorMap.Sample(samplerColorMap, input.UV).rgb, 1.0);
}