#ifndef COMMON
#define COMMON
#include "MathHelper.hlsl"

struct UBO
{
    row_major float4x4 view;
    row_major float4x4 proj;
    row_major float4x4 invVPF;
    float4 cameraPos;
    float4 lightDir;
    float4 lightColor;//rgb for color a for intensity
};
cbuffer PerframeUbo : register(b0) { UBO PerframeUbo; }
#endif
