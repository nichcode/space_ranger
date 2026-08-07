
// binding 0 set 0
Texture2D textures[16] : register(t0, space0);

// binding 1 set 0
SamplerState samp : register(s0, space0);

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    nointerpolation uint textureIndex : TEXCOORD1;
};

float4 main(PSInput input) : SV_Target
{
    uint index = (uint)input.textureIndex;
    return textures[index].Sample(samp, input.uv);
}