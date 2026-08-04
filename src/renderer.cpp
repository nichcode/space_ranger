
#include "renderer.h"
#include "glm/glm.hpp"

#define MAX_QUADS 10000
#define MAX_VERTICES MAX_QUADS * 4
#define MAX_INDICES MAX_QUADS * 6

struct Vertex {
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec4 color;
    float texIndex;
};

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

    m_Device = device;
    return true;
}

void Renderer::shutdown()
{
    palDestroyBuffer(m_IndexBuffer);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        palDestroyBuffer(m_Frames[i].vertexBuffer);
        palDestroyBuffer(m_Frames[i].uploadBuffer);
    }
}