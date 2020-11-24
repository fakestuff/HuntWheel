struct VSOutput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};

float4 main(VSOutput input) : SV_TARGET
{
  return float4(input.Pos.xyz, 1.0);
}