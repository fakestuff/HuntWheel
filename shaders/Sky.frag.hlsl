#include "Common.hlsl"
struct VSOutput
{
    float4 Pos : SV_POSITION;
    float4 WorldPos : TEXCOORD0;
    
};

float4 main(VSOutput input) : SV_TARGET
{
    float3 viewDir = normalize(input.WorldPos.xyz - PerframeUbo.cameraPos.xyz);
    return float4(viewDir, 1);
}