Texture2D textureColorMap : register(t1);
SamplerState samplerColorMap : register(s1);

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 Color : COLOR0;
    float2 UV : TEXCOORD0;
};

float4 main(VSOutput input) : SV_TARGET
{
  return float4(textureColorMap.Sample(samplerColorMap, input.UV).rgb, 1.0);
}