
#include "renderer.h"

#define MAX_QUADS 10000
#define MAX_VERTICES MAX_QUADS * 4
#define MAX_INDICES MAX_QUADS * 6

static glm::vec4 quadVertices[4];

static inline uint32_t align(
    uint32_t value,
    uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

bool Renderer::initialize(
    PalDevice* device, 
    PalCommandBuffer* cmdBuffer, 
    PalQueue* queue)
{
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
        PalResult result = palCreateBuffer(device, &bufferCreateInfo, &frame->vertexBuffer);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create buffer");
            return false;
        }

        // create upload buffer
        bufferCreateInfo.memoryUsage = PAL_BUFFER_MEMORY_USAGE_AUTO_CPU_UPLOAD;
        bufferCreateInfo.usages = PAL_BUFFER_USAGE_TRANSFER_SRC;
        result = palCreateBuffer(device, &bufferCreateInfo, &frame->uploadBuffer);
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
    PalResult result = palCreateBuffer(device, &bufferCreateInfo, &m_IndexBuffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create buffer");
        return false;
    }

    // create staging buffer for index buffer
    PalBuffer* stagingBuffer = nullptr;
    bufferCreateInfo.memoryUsage = PAL_BUFFER_MEMORY_USAGE_AUTO_CPU_UPLOAD;
    bufferCreateInfo.usages = PAL_BUFFER_USAGE_TRANSFER_SRC;
    bufferCreateInfo.size = sizeof(uint32_t) * MAX_INDICES;
    result = palCreateBuffer(device, &bufferCreateInfo, &stagingBuffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create buffer");
        return false;
    }

    // create a fence
    PalFence* fence = nullptr;
    result = palCreateFence(device, PAL_FALSE, &fence);
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
    result = palCmdBegin(cmdBuffer, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to begin command buffer");
        return false;
    }

    PalBufferCopyInfo bufferCopyInfo = {0};
    bufferCopyInfo.size = bufferCreateInfo.size;
    palCmdCopyBuffer(cmdBuffer, m_IndexBuffer, stagingBuffer, &bufferCopyInfo);

    result = palCmdEnd(cmdBuffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to end command buffer");
        return false;
    }

    PalCommandBufferSubmitInfo submitInfo = {0};
    submitInfo.cmdBuffer = cmdBuffer;
    submitInfo.fence = fence;
    result = palSubmitCommandBuffer(queue, &submitInfo);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to submit command buffer");
        return false;
    }

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

    m_Device = device;
    m_CurrentFrame = nullptr;
    return true;
}

void Renderer::shutdown()
{
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