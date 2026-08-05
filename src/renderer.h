
#pragma once

#include "pal2/pal_graphics.h"
#include "helper.h"
#include "glm/glm.hpp"

struct RendererInitInfo {
    PalDevice* device;
    PalCommandBuffer* cmdBuffer;
    PalQueue* queue;
    PalShaderFormats shaderFormats;
    PalFormat attachmentFormat;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec4 color;
    uint32_t texIndex;
};

struct Frame {
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t offset;
    Vertex* ptr;
    PalBuffer* vertexBuffer;
    PalBuffer* uploadBuffer;
};

class Renderer {
public:
    bool initialize(RendererInitInfo* info); 
    void shutdown();
    void begin(uint32_t frameIndex);
    void end(PalCommandBuffer* cmdBuffer);

private:
    void reset(Frame* frame);
    void flush(PalCommandBuffer* cmdBuffer);

private:
    PalDevice* m_Device;
    PalBuffer* m_IndexBuffer;
    Frame* m_CurrentFrame;
    Frame m_Frames[MAX_FRAMES_IN_FLIGHT];

    PalDescriptorSetLayout* m_DescriptorSetLayout;
    PalDescriptorPool* m_DescriptorPool;
    PalDescriptorSet* m_DescriptorSet;
    PalPipelineLayout* m_PipelineLayout;
    PalPipeline* m_QuadPipeline;
};