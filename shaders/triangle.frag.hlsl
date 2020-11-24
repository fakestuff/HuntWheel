struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 Color : COLOR0;
};

float4 main(VSOutput input) : SV_TARGET
{
  return float4(input.Color, 1.0);
}