
struct VSOutput
{
    float4 Pos : SV_POSITION;
[[vk::location(0)]] float3 Color : COLOR0;
};

static float2 positions[3] = {
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
};

static float3 colors[3] = {
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0)
};



VSOutput main(uint vid : SV_VertexID)
{
	VSOutput output = (VSOutput)0;
	output.Color = colors[vid];
	output.Pos =  float4(positions[vid],0.0,1.0);
	return output;
}