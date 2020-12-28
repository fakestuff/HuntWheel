struct VSInput
{
    float3 Pos : POSITION0;
    float3 Normal  : NORMAL0;
    float3 Tangent : TANGENT0;
    float2 UV : TEXCOORD0;
};


struct VSOutput
{
    float4 Pos : SV_POSITION;
    float2 UV: TEXCOORD0;
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
    float4 pos =  mul(mul(mul(float4(input.Pos.xyz, 1.0), pushConsts.model),ubo.view),ubo.proj); // row major
    output.Pos =  pos;
    output.UV = float2(input.UV.x,1.0-input.UV.y);
	return output;
}