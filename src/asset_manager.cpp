
#include "asset_manager.h"
#include "helper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

struct Data {
    PalBuffer* buffer = nullptr;
    PalFence* fence = nullptr;
    PalQueue* queue = nullptr;
    PalCommandPool* cmdPool;
    PalCommandBuffer* cmdBuffer = nullptr;
    PalDevice* device;
    uint32_t size;
    void* ptr;
};

static Data s_Data;

void AssetManager::initialize(PalDevice* device)
{
    PalResult result = palCreateQueue(device, PAL_QUEUE_TYPE_COPY, &s_Data.queue);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create queue");
        DEBUG_BREAK();
        return;
    }

    result = palCreateCommandPool(device, s_Data.queue, &s_Data.cmdPool);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create command pool");
        palDestroyQueue(s_Data.queue);
        DEBUG_BREAK();
        return;
    }

    result = palAllocateCommandBuffer(
        device, 
        s_Data.cmdPool, 
        PAL_COMMAND_BUFFER_TYPE_PRIMARY, 
        &s_Data.cmdBuffer);

    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create command pool");
        palDestroyCommandPool(s_Data.cmdPool);
        palDestroyQueue(s_Data.queue);
        DEBUG_BREAK();
        return;
    }

    result = palCreateFence(device, PAL_FALSE, &s_Data.fence);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create command pool");
        palFreeCommandBuffer(s_Data.cmdBuffer);
        palDestroyCommandPool(s_Data.cmdPool);
        palDestroyQueue(s_Data.queue);
        DEBUG_BREAK();
        return;
    }

    PalBufferCreateInfo createInfo = {0};
    createInfo.memoryUsage = PAL_BUFFER_MEMORY_USAGE_AUTO_CPU_UPLOAD;
    createInfo.usages = PAL_BUFFER_USAGE_TRANSFER_SRC;
    createInfo.size = 1024 * 1024 * 8;
    s_Data.size = createInfo.size;

    result = palCreateBuffer(device, &createInfo, &s_Data.buffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create command pool");
        palDestroyFence(s_Data.fence);
        palFreeCommandBuffer(s_Data.cmdBuffer);
        palDestroyCommandPool(s_Data.cmdPool);
        palDestroyQueue(s_Data.queue);
        DEBUG_BREAK();
        return;
    }

    result = palMapBuffer(s_Data.buffer, 0, s_Data.size, &s_Data.ptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to map buffer");
        palDestroyBuffer(s_Data.buffer);
        palDestroyFence(s_Data.fence);
        palFreeCommandBuffer(s_Data.cmdBuffer);
        palDestroyCommandPool(s_Data.cmdPool);
        palDestroyQueue(s_Data.queue);
        DEBUG_BREAK();
        return;
    }

    s_Data.device = device;
}

void AssetManager::shutdown()
{
    palUnmapBuffer(s_Data.buffer);
    palDestroyBuffer(s_Data.buffer);
    palDestroyFence(s_Data.fence);
    palFreeCommandBuffer(s_Data.cmdBuffer);
    palDestroyCommandPool(s_Data.cmdPool);
    palDestroyQueue(s_Data.queue);
}

Texture* AssetManager::loadTexture(const char* path)
{
    int width = 0, height = 0, channels = 0;
	stbi_uc* data = stbi_load(path, &width, &height, &channels, 0);
    if (!data) {
        return nullptr;
    }

    PalFormat format = PAL_FORMAT_R8G8B8A8_SRGB;
    if (channels == 3) {
        format = PAL_FORMAT_R8G8B8_SRGB;
    }

    Texture* texture = (Texture*)palAllocate(nullptr, sizeof(Texture), 0);
    if (!texture) {
        return nullptr;
    }

    PalImageCreateInfo createInfo = {0};
    createInfo.width = width;
    createInfo.height = height;
    createInfo.arrayLayerCount = 1;
    createInfo.depth = 1;
    createInfo.format = format;

    createInfo.memoryUsage = PAL_IMAGE_MEMORY_USAGE_AUTO_GPU_ONLY;
    createInfo.mipLevelCount = 1;
    createInfo.sampleCount = PAL_SAMPLE_COUNT_1;
    createInfo.type = PAL_IMAGE_TYPE_2D;
    createInfo.usages = PAL_IMAGE_USAGE_SAMPLED | PAL_IMAGE_USAGE_TRANSFER_DST;

    PalResult result = palCreateImage(s_Data.device, &createInfo, &texture->image);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create texture");
        palFree(nullptr, texture);
        return nullptr;
    }

    PalBufferImageCopyInfo copyInfo = {0};
    copyInfo.ImageArrayLayerCount = 1;
    copyInfo.imageWidth = width;
    copyInfo.imageHeight = height;
    copyInfo.imageDepth = 1;

    PalImageStagingRequirements stagingReq = {0};
    palComputeImageStagingRequirements(
        s_Data.device,
        format,
        &copyInfo,
        &stagingReq);

    copyInfo.bufferRowLength = stagingReq.bufferRowLength;
    copyInfo.bufferImageHeight = stagingReq.bufferImageHeight;
    palWriteImageStaging(s_Data.device, format, &copyInfo, data, s_Data.ptr);

    result = palCmdBegin(s_Data.cmdBuffer, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to begin command buffer");
        palDestroyImage(texture->image);
        palFree(nullptr, texture);
        return nullptr;
    }

    PalBarrierInfo barrierInfo = {0};
    barrierInfo.oldState = PAL_USAGE_STATE_UNDEFINED;
    barrierInfo.srcStages = PAL_PIPELINE_STAGE_NONE;
    barrierInfo.newState = PAL_USAGE_STATE_TRANSFER_WRITE;
    barrierInfo.dstStages = PAL_PIPELINE_STAGE_TRANSFER;

    PalImageSubresourceRange range = {0};
    range.startMipLevel = 0;
    range.startArrayLayer = 0;
    range.mipLevelCount = 1;
    range.layerArrayCount = 1;

    palCmdImageBarrier(s_Data.cmdBuffer, texture->image, &range, &barrierInfo);
    palCmdCopyBufferToImage(s_Data.cmdBuffer, texture->image, s_Data.buffer, &copyInfo);

    // transition the image to shader read state
    barrierInfo.oldState = PAL_USAGE_STATE_TRANSFER_WRITE;
    barrierInfo.srcStages = PAL_PIPELINE_STAGE_TRANSFER;
    barrierInfo.newState = PAL_USAGE_STATE_SHADER_READ;
    barrierInfo.dstStages = PAL_PIPELINE_STAGE_FRAGMENT_SHADER;
    palCmdImageBarrier(s_Data.cmdBuffer, texture->image, &range, &barrierInfo);

    result = palCmdEnd(s_Data.cmdBuffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to end command buffer");
        palDestroyImage(texture->image);
        palFree(nullptr, texture);
        return nullptr;
    }

    PalCommandBufferSubmitInfo submitInfo = {0};
    submitInfo.cmdBuffer = s_Data.cmdBuffer;
    submitInfo.fence = s_Data.fence;
    submitInfo.waitStages = PAL_PIPELINE_STAGE_TRANSFER;

    result = palSubmitCommandBuffer(s_Data.queue, &submitInfo);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to submit command buffer");
        palDestroyImage(texture->image);
        palFree(nullptr, texture);
        return nullptr;
    }

    PalImageViewCreateInfo imageViewCreateInfo = {0};
    imageViewCreateInfo.type = PAL_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.subresourceRange = range;
    imageViewCreateInfo.format = format;

    result = palCreateImageView(
        s_Data.device,
        texture->image,
        &imageViewCreateInfo,
        &texture->imageView);

    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create image view");
        palDestroyImage(texture->image);
        palFree(nullptr, texture);
        return nullptr;
    }

    stbi_image_free(data);
    return texture;
}

void AssetManager::destroyTexture(Texture* texture)
{
    palDestroyImageView(texture->imageView);
    palDestroyImage(texture->image);
}