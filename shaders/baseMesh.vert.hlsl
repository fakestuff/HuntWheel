
struct VSInput
{
    float3 pos : POSITION0;
    float3 normal : NORMAL0;
    float3 tangent : TANGENT0;
    float2 uv  : TEXCOORD0;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

struct PushConsts
{
	row_major float4x4 model;
};
[[vk::push_constant]]PushConsts pushConsts;

struct UBO
{
	row_major float4x4 view;
	row_major float4x4 proj;
};
cbuffer ubo : register(b0) { UBO ubo; }

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;
	output.pos =  float4(input.pos,1.0);
	output.uv = input.uv;
	return output;
}