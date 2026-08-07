
#version 450 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in uint aTextureIndex;

layout(location = 0) out vec2 vUv;
layout(location = 1) out flat uint vTextureIndex;

layout(push_constant) uniform PushConstants
{
    mat4 uViewProjection;
} pc;

void main()
{
    gl_Position = pc.uViewProjection * vec4(aPos, 0.0, 1.0);
    vUv = aUv;
    vTextureIndex = aTextureIndex;
}