struct VSOutput
{
    float4 Pos : SV_POSITION;
};

float4 main(VSOutput input) : SV_TARGET
{
    float4 c = float4(input.Pos.xy/1024,0,1);
    return c;
}