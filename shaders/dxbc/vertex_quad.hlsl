
struct VSInput 
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
    nointerpolation uint textureIndex : TEXCOORD1;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    nointerpolation uint textureIndex : TEXCOORD1;
};

struct PushData
{
    float4x4 viewProjection;
};

ConstantBuffer<PushData> pc : register(b0);

VSOutput main(VSInput input)
{
    VSOutput output;
    output.pos = mul(pc.viewProjection, float4(input.pos, 1.0));
    output.uv = input.uv;
    output.color = input.color;
    output.textureIndex = input.textureIndex;
    return output;
}