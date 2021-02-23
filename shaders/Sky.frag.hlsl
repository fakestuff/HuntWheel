struct VSOutput
{
    float4 Pos : SV_POSITION;
    float4 WorldPos : TEXCOORD0;
    
};

struct UBO
{
	row_major float4x4 view;
	row_major float4x4 proj;
    row_major float4x4 invVPF;
    float4 cameraPos;
};
cbuffer ubo : register(b0) { UBO ubo; }

float4 main(VSOutput input) : SV_TARGET
{
    float3 viewDir = normalize(input.WorldPos.xyz - ubo.cameraPos.xyz);
    return float4(viewDir, 1);
}