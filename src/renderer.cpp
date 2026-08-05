
#include "renderer.h"
#include <stdio.h>

#define MAX_QUADS 10000
#define MAX_VERTICES MAX_QUADS * 4
#define MAX_INDICES MAX_QUADS * 6

static glm::vec4 quadVertices[4];

struct PushConstant
{
    glm::mat4 viewProjection;
};

static inline uint32_t align(
    uint32_t value,
    uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

static PalBool readFile(
    const char* filename,
    void* buffer,
    uint32_t* size)
{
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return PAL_FALSE;
    }

    fseek(file, 0, SEEK_END);
    uint32_t tmpSize = (uint32_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    if (buffer) {
        tmpSize = *size;
        size_t read = fread(buffer, 1, tmpSize, file);
        if ((uint32_t)read != tmpSize) {
            return PAL_FALSE;
        }
    }

    fclose(file);
    *size = tmpSize;
    return PAL_TRUE;
}

bool Renderer::initialize(RendererInitInfo* info)
{
    // load shader files
    void* vertexCode = nullptr;
    void* fragmentCode = nullptr;
    uint32_t vertexCodeSize = 0;
    uint32_t fragmentCodeSize = 0;
    const char* vertexShaderPath = nullptr;
    const char* fragmentShaderPath = nullptr;

    if (info->shaderFormats & PAL_SHADER_FORMAT_SPIRV) {
        vertexShaderPath = "shaders/spirv/vertex_quad.glsl";
        fragmentShaderPath = "shaders/spirv/fragment_quad.glsl";

    } else if (info->shaderFormats & PAL_SHADER_FORMAT_DXBC) {
        vertexShaderPath = "shaders/dxbc/vertex_quad.cso";
        fragmentShaderPath = "shaders/dxbc/fragment_quad.cso";

    } else {
        palLog(nullptr, "Failed to find a supported shader format");
        return false;
    }

    readFile(vertexShaderPath, nullptr, &vertexCodeSize);
    readFile(fragmentShaderPath, nullptr, &fragmentCodeSize);
    if (!vertexCodeSize || !fragmentCodeSize) {
        palLog(nullptr, "Failed to read shader files");
        return false;
    }

    vertexCode = palAllocate(nullptr, vertexCodeSize, 0);
    fragmentCode = palAllocate(nullptr, fragmentCodeSize, 0);
    if (!vertexCode || !fragmentCode) {
        palLog(nullptr, "Failed to allocate memory");
        return false;
    }

    readFile(vertexShaderPath, vertexCode, &vertexCodeSize);
    readFile(fragmentShaderPath, fragmentCode, &fragmentCodeSize);

    // build indices
    uint32_t* indices = nullptr;
    indices = (uint32_t*)palAllocate(nullptr, sizeof(uint32_t) * MAX_INDICES, 0);
    if (!indices) {
        return false;
    }

    for (int i = 0; i < MAX_QUADS; i++) {
        uint32_t offset = i * 4;

        indices[i * 6 + 0] = offset + 0;
        indices[i * 6 + 1] = offset + 1;
        indices[i * 6 + 2] = offset + 2;

        indices[i * 6 + 3] = offset + 2;
        indices[i * 6 + 4] = offset + 3;
        indices[i * 6 + 5] = offset + 0;
    }

    PalBufferCreateInfo bufferCreateInfo = {0};
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        Frame* frame = &m_Frames[i];

        bufferCreateInfo.memoryUsage = PAL_BUFFER_MEMORY_USAGE_AUTO_GPU_ONLY;
        bufferCreateInfo.usages = PAL_BUFFER_USAGE_VERTEX | PAL_BUFFER_USAGE_TRANSFER_DST;
        bufferCreateInfo.size = sizeof(Vertex) * MAX_VERTICES;
        PalResult result = palCreateBuffer(info->device, &bufferCreateInfo, &frame->vertexBuffer);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create buffer");
            return false;
        }

        // create upload buffer
        bufferCreateInfo.memoryUsage = PAL_BUFFER_MEMORY_USAGE_AUTO_CPU_UPLOAD;
        bufferCreateInfo.usages = PAL_BUFFER_USAGE_TRANSFER_SRC;
        result = palCreateBuffer(info->device, &bufferCreateInfo, &frame->uploadBuffer);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create buffer");
            return false;
        }

        void* ptr = nullptr;
        result = palMapBuffer(frame->uploadBuffer, 0, bufferCreateInfo.size, &ptr);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to map buffer");
            return false;
        }

        frame->ptr = (Vertex*)ptr;
        frame->vertexCount = 0;
        frame->indexCount = 0;
        frame->offset = 0;
    }

    // create index buffer
    bufferCreateInfo.memoryUsage = PAL_BUFFER_MEMORY_USAGE_AUTO_GPU_ONLY;
    bufferCreateInfo.usages = PAL_BUFFER_USAGE_INDEX | PAL_BUFFER_USAGE_TRANSFER_DST;
    bufferCreateInfo.size = sizeof(uint32_t) * MAX_INDICES;
    PalResult result = palCreateBuffer(info->device, &bufferCreateInfo, &m_IndexBuffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create buffer");
        return false;
    }

    // create staging buffer for index buffer
    PalBuffer* stagingBuffer = nullptr;
    bufferCreateInfo.memoryUsage = PAL_BUFFER_MEMORY_USAGE_AUTO_CPU_UPLOAD;
    bufferCreateInfo.usages = PAL_BUFFER_USAGE_TRANSFER_SRC;
    bufferCreateInfo.size = sizeof(uint32_t) * MAX_INDICES;
    result = palCreateBuffer(info->device, &bufferCreateInfo, &stagingBuffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create buffer");
        return false;
    }

    // create a fence
    PalFence* fence = nullptr;
    result = palCreateFence(info->device, PAL_FALSE, &fence);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create fence");
        return false;
    }

    // copy indices to staging buffer
    void* ptr = nullptr;
    result = palMapBuffer(stagingBuffer, 0, bufferCreateInfo.size, &ptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to map buffer");
        return false;
    }

    memcpy(ptr, indices, bufferCreateInfo.size);
    palUnmapBuffer(stagingBuffer);

    // copy staging buffer to index buffer
    result = palCmdBegin(info->cmdBuffer, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to begin command buffer");
        return false;
    }

    PalBufferCopyInfo bufferCopyInfo = {0};
    bufferCopyInfo.size = bufferCreateInfo.size;
    palCmdCopyBuffer(info->cmdBuffer, m_IndexBuffer, stagingBuffer, &bufferCopyInfo);

    result = palCmdEnd(info->cmdBuffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to end command buffer");
        return false;
    }

    PalCommandBufferSubmitInfo submitInfo = {0};
    submitInfo.cmdBuffer = info->cmdBuffer;
    submitInfo.fence = fence;
    result = palSubmitCommandBuffer(info->queue, &submitInfo);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to submit command buffer");
        return false;
    }

    // create descriptor set layout
    PalDescriptorSetLayoutBinding bindings[2] = {
        { 16, PAL_DESCRIPTOR_TYPE_SAMPLED_IMAGE },
        { 1, PAL_DESCRIPTOR_TYPE_SAMPLER }
    };

    PalDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {0};
    descriptorSetLayoutCreateInfo.bindingCount = 2;
    descriptorSetLayoutCreateInfo.bindings = bindings;
    result = palCreateDescriptorSetLayout(
        info->device, 
        &descriptorSetLayoutCreateInfo, 
        &m_DescriptorSetLayout);

    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create descriptor set layout");
        return false;
    }

    // create descriptor pool
    PalDescriptorPoolBindingSize bindingSizes[2] = {
        { 16, PAL_DESCRIPTOR_TYPE_SAMPLED_IMAGE },
        { 1, PAL_DESCRIPTOR_TYPE_SAMPLER }
    };

    PalDescriptorPoolCreateInfo descriptorPoolCreateInfo = {0};
    descriptorPoolCreateInfo.bindingSizeCount = 2;
    descriptorPoolCreateInfo.bindingSizes = bindingSizes;
    descriptorPoolCreateInfo.maxDescriptorSets = 1;
    result = palCreateDescriptorPool(info->device, &descriptorPoolCreateInfo, &m_DescriptorPool);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create descriptor pool");
        return false;
    }

    // allocate descriptor set
    result = palAllocateDescriptorSet(
        info->device, 
        m_DescriptorPool, 
        m_DescriptorSetLayout, 
        &m_DescriptorSet);

    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to allocate descriptor set");
        return false;
    }

    // create shaders
    PalShader* vertexShader = nullptr;
    PalShader* fragmentShader = nullptr;

    PalShaderEntryInfo vertexEntryInfo = {0};
    vertexEntryInfo.entryName = "main";
    vertexEntryInfo.stage = PAL_SHADER_STAGE_VERTEX;

    PalShaderEntryInfo fragmentEntryInfo = {0};
    fragmentEntryInfo.entryName = "main";
    fragmentEntryInfo.stage = PAL_SHADER_STAGE_FRAGMENT;

    PalShaderCreateInfo shaderCreateInfo = {0};
    shaderCreateInfo.code = vertexCode;
    shaderCreateInfo.codeSize = vertexCodeSize;
    shaderCreateInfo.entryCount = 1;
    shaderCreateInfo.entries = &vertexEntryInfo;
    result = palCreateShader(info->device, &shaderCreateInfo, &vertexShader);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create shader");
        return false;
    }

    // create fragment shader
    shaderCreateInfo.code = fragmentCode;
    shaderCreateInfo.codeSize = fragmentCodeSize;
    shaderCreateInfo.entryCount = 1;
    shaderCreateInfo.entries = &fragmentEntryInfo;
    result = palCreateShader(info->device, &shaderCreateInfo, &fragmentShader);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create shader");
        return false;
    }

    palFree(nullptr, vertexCode);
    palFree(nullptr, fragmentCode);

    // create pipeline layout
    PalPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {0};
    pipelineLayoutCreateInfo.descriptorSetLayoutCount = 1;
    pipelineLayoutCreateInfo.descriptorSetLayouts = &m_DescriptorSetLayout;
    pipelineLayoutCreateInfo.usePushConstant = PAL_TRUE;
    pipelineLayoutCreateInfo.pushConstantInfo.size = sizeof(PushConstant);
    result = palCreatePipelineLayout(info->device, &pipelineLayoutCreateInfo, &m_PipelineLayout);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create pipeline layout");
        return false;
    }

    // create pipeline
    PalVertexAttribute vertexAttributes[4] = {
        { PAL_VERTEX_SEMANTIC_ID_POSITION, PAL_VERTEX_TYPE_FLOAT3 },
        { PAL_VERTEX_SEMANTIC_ID_TEXCOORD, PAL_VERTEX_TYPE_FLOAT2 },
        { PAL_VERTEX_SEMANTIC_ID_COLOR, PAL_VERTEX_TYPE_FLOAT4 },
        { PAL_VERTEX_SEMANTIC_ID_TEXCOORD, PAL_VERTEX_TYPE_UINT32 } // TEXCOORD1
    };

    PalVertexLayout vertexLayout = {0};
    vertexLayout.attributeCount = 4;
    vertexLayout.attributes = vertexAttributes;
    vertexLayout.binding = 0;
    vertexLayout.type = PAL_VERTEX_LAYOUT_TYPE_PER_VERTEX;

    PalRenderingLayoutInfo renderingLayoutInfo = {0};
    renderingLayoutInfo.colorAttachentCount = 1;
    renderingLayoutInfo.colorAttachmentsFormat = &info->attachmentFormat;
    renderingLayoutInfo.sampleCount = PAL_SAMPLE_COUNT_1;
    renderingLayoutInfo.viewCount = 1;

    // color blend attachment
    PalColorBlendAttachment blendAttachment = {0};
    blendAttachment.colorWriteMask |= PAL_COLOR_MASK_RED;
    blendAttachment.colorWriteMask |= PAL_COLOR_MASK_GREEN;
    blendAttachment.colorWriteMask |= PAL_COLOR_MASK_BLUE;
    blendAttachment.colorWriteMask |= PAL_COLOR_MASK_ALPHA;

    PalGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {0};
    graphicsPipelineCreateInfo.colorBlendAttachmentCount = 1;
    graphicsPipelineCreateInfo.colorBlendAttachments = &blendAttachment;
    graphicsPipelineCreateInfo.pipelineLayout = m_PipelineLayout;

    graphicsPipelineCreateInfo.renderingLayout = &renderingLayoutInfo;
    graphicsPipelineCreateInfo.topology = PAL_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    graphicsPipelineCreateInfo.vertexLayoutCount = 1;
    graphicsPipelineCreateInfo.vertexLayouts = &vertexLayout;

    PalShader* shaders[2] = { vertexShader, fragmentShader };
    graphicsPipelineCreateInfo.shaders = shaders;
    graphicsPipelineCreateInfo.shaderCount = 2;

    result = palCreateGraphicsPipeline(info->device, &graphicsPipelineCreateInfo, &m_QuadPipeline);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create graphics pipeline");
        return false;
    }

    palDestroyShader(vertexShader);
    palDestroyShader(fragmentShader);


    // wait for the transfer
    result = palWaitFence(fence, PAL_INFINITE);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to wait fence");
        return false;
    }

    palDestroyBuffer(stagingBuffer);
    palDestroyFence(fence);
    palFree(nullptr, indices);

    quadVertices[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
    quadVertices[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
    quadVertices[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
    quadVertices[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

    // reset the command buffer
    result = palResetCommandBuffer(info->cmdBuffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to reset command buffer");
        return false;
    }

    m_Device = info->device;
    m_CurrentFrame = nullptr;
    return true;
}

void Renderer::shutdown()
{
    palDestroyPipeline(m_QuadPipeline);
    palDestroyPipelineLayout(m_PipelineLayout);
    palDestroyDescriptorPool(m_DescriptorPool);
    palDestroyDescriptorSetLayout(m_DescriptorSetLayout);
    palDestroyBuffer(m_IndexBuffer);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        palDestroyBuffer(m_Frames[i].vertexBuffer);
        palUnmapBuffer(m_Frames[i].uploadBuffer);
        palDestroyBuffer(m_Frames[i].uploadBuffer);
    }
}

void Renderer::begin(uint32_t frameIndex)
{
    m_CurrentFrame = &m_Frames[frameIndex];
    reset(m_CurrentFrame);
}

void Renderer::end(PalCommandBuffer* cmdBuffer)
{
    flush(cmdBuffer);
}

void Renderer::reset(Frame* frame)
{
    frame->offset = 0;
    frame->indexCount = 0;
    frame->vertexCount = 0;
}

void Renderer::flush(PalCommandBuffer* cmdBuffer)
{
    // TODO: bind descriptor set and pipeline

    // copy the upload buffer to the vertex buffer
    uint32_t dataSize = sizeof(Vertex) * m_CurrentFrame->vertexCount;
    PalBufferCopyInfo copyInfo = {0};
    copyInfo.size = dataSize;
    palCmdCopyBuffer(
        cmdBuffer, 
        m_CurrentFrame->vertexBuffer, 
        m_CurrentFrame->uploadBuffer, 
        &copyInfo);

    // put a barrier to make sure the copy is completed before draw
    PalBarrierInfo barrierInfo = {0};
    barrierInfo.oldState = PAL_USAGE_STATE_TRANSFER_WRITE;
    barrierInfo.srcStages = PAL_PIPELINE_STAGE_TRANSFER;
    barrierInfo.newState = PAL_USAGE_STATE_SHADER_READ;
    barrierInfo.dstStages = PAL_PIPELINE_STAGE_VERTEX_SHADER;
    palCmdBufferBarrier(cmdBuffer, m_CurrentFrame->vertexBuffer, &barrierInfo);
    palCmdDrawIndexed(cmdBuffer, m_CurrentFrame->indexCount, 1, 0, 0, 0);
}