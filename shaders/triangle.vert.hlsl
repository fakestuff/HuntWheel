struct VSInput
{
    float3 Pos : POSITION0;
    float3 Color  : COLOR0;
    float2 UV : TEXCOORD0;
};


struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 Color : COLOR0;
    float2 UV: TEXCOORD0;
};

struct UBO
{
	row_major float4x4 model;
	row_major float4x4 view;
	row_major float4x4 proj;
};
cbuffer ubo : register(b0) { UBO ubo; }

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;
	output.Color = input.Color;
    float4 pos =  mul(mul(mul(float4(input.Pos.xyz, 1.0), ubo.model),ubo.view),ubo.proj); // row major
    output.Pos =  pos;
    output.UV = input.UV;
	return output;
}