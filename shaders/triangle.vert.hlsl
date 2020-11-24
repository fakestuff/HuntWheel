struct VSInput
{
    float2 Pos : POSITION0;
    float3 Color  : COLOR0;
};


struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 Color : COLOR0;
};

// static float2 positions[3] = {
//     float2(0.0, -0.5),
//     float2(0.5, 0.5),
//     float2(-0.5, 0.5)
// };

// static float3 colors[3] = {
//     float3(1.0, 0.0, 0.0),
//     float3(0.0, 1.0, 0.0),
//     float3(0.0, 0.0, 1.0)
// };



VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;
	output.Color = input.Color;
	output.Pos =  float4(input.Pos,0.0,1.0);
	return output;
}