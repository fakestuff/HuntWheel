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
};
cbuffer PerframeUbo : register(b0) { UBO PerframeUbo; }
#endif
