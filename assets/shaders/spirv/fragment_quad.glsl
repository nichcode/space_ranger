
#version 450 core

layout(location = 0) in vec2 aUv;
layout(location = 1) in vec4 aColor;
layout(location = 2) in flat uint aTextureIndex;

layout(binding = 0, set = 0) uniform texture2D textures[16];
layout(binding = 1, set = 0) uniform sampler samp;

layout(location = 0) out vec4 vColor;

void main()
{
    uint index = uint(aTextureIndex);
    vColor = aColor * texture(sampler2D(textures[index], samp), aUv);
}