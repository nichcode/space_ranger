
#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec4 aColor;
layout(location = 3) in uint aTextureIndex;

layout(location = 0) out vec2 vUv;
layout(location = 1) out vec4 vColor;
layout(location = 2) out flat uint vTextureIndex;

layout(push_constant) uniform PushConstants
{
    mat4 uViewProjection;
} pc;

void main()
{
    gl_Position = pc.uViewProjection * vec4(aPos, 1.0);
    vColor = aColor;
    vUv = aUv;
    vTextureIndex = aTextureIndex;
}