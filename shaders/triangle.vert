/*struct VSInput
{
[[vk::location(0)]] float3 Pos : POSITION0;
[[vk::location(1)]] float3 Color : COLOR0;
};*/



struct VSOutput
{
    float4 Pos : SV_POSITION;
[[vk::location(0)]] float3 Color : COLOR0;
};

float2 positions[3] = float2[](
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
);

float3 colors[3] = float3[](
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0)
);



VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;
	output.Color = colors[SV_VertexID];
	output.Pos =  float4(positions[SV_VertexID],0.0,1.0);
	return output;
}