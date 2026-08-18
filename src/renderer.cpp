
#include "pal2/pal_system.h"
#include "renderer.h"
#include "helper.h"
#include "asset_manager.h"
#include "glm/gtc/matrix_transform.hpp"

#include <stdio.h>

#define MAX_QUADS 10000
#define MAX_VERTICES MAX_QUADS * 4
#define MAX_INDICES MAX_QUADS * 6
#define MAX_TEXTURE_SLOTS 16

#define min(a, b) (a < b) ? a : b

struct Vertex {
    glm::vec2 pos;
    glm::vec2 uv;
    uint32_t texIndex;
};

struct PushConstant {
    glm::mat4 viewProjection;
};

static glm::vec4 s_Vertices[4];
static glm::vec2 s_TextureCoords[4];
static PalDescriptorImageViewInfo s_TextureSlots[MAX_TEXTURE_SLOTS];
static uint32_t s_TextureSlotIndex = 0;

bool Renderer::initialize(
    PalWindow* window, 
    PalAdapter* adapter, 
    PalDevice* device, 
    uint32_t windowWidth, 
    uint32_t windowHeight)
{
    memset(m_Frames, 0, sizeof(Frame) * MAX_FRAMES_IN_FLIGHT);
    m_Device = device;

    PalPlatformInfo platformInfo = {0};
    PalWindowHandleInfo winHandle = {0};
    palGetWindowHandleInfo(window, &winHandle);
    palGetPlatformInfo(&platformInfo);

    PalAdapterCapabilities adapterCaps = {};
    PalAdapterInfo adapterInfo = {};
    palGetAdapterInfo(adapter, &adapterInfo);
    palGetAdapterCapabilities(adapter, &adapterCaps);

    PalWindowInstanceType windowInstanceType = PAL_WINDOW_INSTANCE_TYPE_XCB;
    if (platformInfo.apiType == PAL_PLATFORM_API_TYPE_WAYLAND) {
        windowInstanceType = PAL_WINDOW_INSTANCE_TYPE_WAYLAND;

    } else if (platformInfo.apiType == PAL_PLATFORM_API_TYPE_X11) {
        windowInstanceType = PAL_WINDOW_INSTANCE_TYPE_X11;

    } else if (platformInfo.apiType == PAL_PLATFORM_API_TYPE_WIN32) {
        windowInstanceType = PAL_WINDOW_INSTANCE_TYPE_WIN32;
    }

    PalResult result = palCreateSurface(
        device,
        winHandle.nativeWindow,
        winHandle.nativeInstance,
        windowInstanceType,
        &m_Surface);

    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create surface");
        DEBUG_BREAK();
        return false;
    }

    PalBool foundQueue = PAL_FALSE;
    for (int i = 0; i < adapterCaps.maxGraphicsQueues; i++) {
        result = palCreateQueue(device, PAL_QUEUE_TYPE_GRAPHICS, &m_Queue);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create queue");
            DEBUG_BREAK();
            return false;
        }

        if (!palCanQueuePresent(m_Queue, m_Surface)) {
            palDestroyQueue(m_Queue);

        } else {
            // found a queue
            foundQueue = PAL_TRUE;
            break;
        }
    }

    if (!foundQueue) {
        palLog(nullptr, "Failed to find a queue that can present to the surface");
        DEBUG_BREAK();
        return false;
    }

    PalSurfaceCapabilities surfaceCaps = {0};
    palGetSurfaceCapabilities(device, m_Surface, &surfaceCaps);

    PalSwapchainCreateInfo swapchainCreateInfo = {0};
    swapchainCreateInfo.clipped = PAL_TRUE;
    swapchainCreateInfo.compositeAlpha = PAL_COMPOSITE_ALPHA_OPAQUE;
    swapchainCreateInfo.height = windowWidth;
    swapchainCreateInfo.width = windowHeight;
    swapchainCreateInfo.imageArrayLayerCount = 1;
    swapchainCreateInfo.presentMode = PAL_PRESENT_MODE_FIFO;
    swapchainCreateInfo.format = PAL_SURFACE_FORMAT_BGRA8_UNORM_SRGB_NONLINEAR;

    // rare but possible on andriod
    if (windowWidth > surfaceCaps.maxImageWidth) {
        swapchainCreateInfo.width = surfaceCaps.maxImageWidth / 2;
    }

    if (windowHeight > surfaceCaps.maxImageHeight) {
        swapchainCreateInfo.height = surfaceCaps.maxImageHeight / 2;
    }

    swapchainCreateInfo.imageCount = surfaceCaps.minImageCount;
    if (swapchainCreateInfo.imageCount == 1) {
        swapchainCreateInfo.imageCount++;
        if (surfaceCaps.maxImageCount < 2) {
            palLog(nullptr, "Surface does not support double buffers");
            DEBUG_BREAK();
            return false;
        }
    }

    result = palCreateSwapchain(
        device, 
        m_Queue, 
        m_Surface, 
        &swapchainCreateInfo, 
        &m_Swapchain);

    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create swapchain");
        palDestroySurface(m_Surface);
        DEBUG_BREAK();
        return false;
    }

    m_ImageCount = swapchainCreateInfo.imageCount;
    m_ImageViews = (PalImageView**)palAllocate(
        nullptr, 
        sizeof(PalImageView*) * m_ImageCount, 
        0);

    m_InFlightImages = (PalFence**)palAllocate(
        nullptr, 
        sizeof(PalFence*) * m_ImageCount, 
        0);

    m_RenderFinishedSemaphores = (PalSemaphore**)palAllocate(
        nullptr, 
        sizeof(PalSemaphore*) * m_ImageCount, 
        0);

    if (!m_ImageViews || !m_InFlightImages || !m_RenderFinishedSemaphores) {
        palLog(nullptr, "Failed to allocate memory");
        palDestroySwapchain(m_Swapchain);
        palDestroySurface(m_Surface);
        DEBUG_BREAK();
        return false;
    }

    PalImageInfo imageInfo;
    palGetImageInfo(palGetSwapchainImage(m_Swapchain, 0), &imageInfo);

    PalImageViewCreateInfo imageViewCreateInfo = {0};
    imageViewCreateInfo.type = PAL_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.subresourceRange.layerArrayCount = 1;
    imageViewCreateInfo.subresourceRange.mipLevelCount = 1;
    imageViewCreateInfo.subresourceRange.startArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.startMipLevel = 0;
    imageViewCreateInfo.format = imageInfo.format;

    for (int i = 0; i < m_ImageCount; i++) {
        PalImage* image = palGetSwapchainImage(m_Swapchain, i);
        if (!image) {
            palLog(nullptr, "Failed to get swapchain image");
            palDestroySwapchain(m_Swapchain);
            palDestroySurface(m_Surface);
            DEBUG_BREAK();
            return false;
        }

        result = palCreateImageView(device, image, &imageViewCreateInfo, &m_ImageViews[i]);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create image view");
            palDestroySwapchain(m_Swapchain);
            palDestroySurface(m_Surface);
            DEBUG_BREAK();
            return false;
        }

        // create render finished semaphores
        result = palCreateSemaphore(device, PAL_FALSE, &m_RenderFinishedSemaphores[i]);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create semaphore");
            palDestroySwapchain(m_Swapchain);
            palDestroySurface(m_Surface);
            DEBUG_BREAK();
            return false;
        }

        m_InFlightImages[i] = nullptr;
    }

    result = palCreateCommandPool(device, m_Queue, &m_CmdPool);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create command pool");
        DEBUG_BREAK();
        return false;
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        Frame* frame = &m_Frames[i];

        result = palCreateSemaphore(
            device, 
            PAL_FALSE, 
            &frame->semaphore);

        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create semaphore");
            DEBUG_BREAK();
            return false;
        }

        result = palCreateFence(device, PAL_TRUE, &frame->fence);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create fence");
            DEBUG_BREAK();
            return false;
        }

        result = palAllocateCommandBuffer(
            device,
            m_CmdPool,
            PAL_COMMAND_BUFFER_TYPE_PRIMARY,
            &frame->cmdBuffer);

        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to allocate command buffer");
            DEBUG_BREAK();
            return false;
        }
    }

    PalDescriptorSetLayoutBinding bindings[2] = {
        { 16, PAL_DESCRIPTOR_TYPE_SAMPLED_IMAGE },
        { 1, PAL_DESCRIPTOR_TYPE_SAMPLER }
    };

    PalDescriptorPoolBindingSize bindingSizes[2] = {
        { 16, PAL_DESCRIPTOR_TYPE_SAMPLED_IMAGE },
        { 1, PAL_DESCRIPTOR_TYPE_SAMPLER }
    };

    PalDescriptorSetLayoutCreateInfo layoutCreateInfo = {0};
    layoutCreateInfo.bindingCount = 2;
    layoutCreateInfo.bindings = bindings;

    result = palCreateDescriptorSetLayout(
        device, 
        &layoutCreateInfo, 
        &m_DescriptorSetLayout);

    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create descriptor set layout");
        DEBUG_BREAK();
        return false;
    }

    PalDescriptorPoolCreateInfo poolCreateInfo = {0};
    poolCreateInfo.bindingSizeCount = 2;
    poolCreateInfo.bindingSizes = bindingSizes;
    poolCreateInfo.maxDescriptorSets = 1;

    result = palCreateDescriptorPool(device, &poolCreateInfo, &m_DescriptorPool);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create descriptor pool");
        DEBUG_BREAK();
        return false;
    }

    result = palAllocateDescriptorSet(
        device, 
        m_DescriptorPool, 
        m_DescriptorSetLayout, 
        &m_DescriptorSet);

    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to allocate descriptor set");
        DEBUG_BREAK();
        return false;
    }

    const char* sources[2];
    if (adapterInfo.shaderFormats & PAL_SHADER_FORMAT_SPIRV) {
        sources[0] = "assets/shaders/spirv/vertex_quad.spv";
        sources[1] = "assets/shaders/spirv/fragment_quad.spv";

    } else if (adapterInfo.shaderFormats & PAL_SHADER_FORMAT_DXBC) {
        sources[0] = "assets/shaders/dxbc/vertex_quad.cso";
        sources[1] = "assets/shaders/dxbc/fragment_quad.cso";

    } else {
        palLog(nullptr, "Failed to find a supported shader format");
        DEBUG_BREAK();
        return false;
    }

    PalShader* shaders[2];
    PalShaderStage stages[2] = { PAL_SHADER_STAGE_VERTEX, PAL_SHADER_STAGE_FRAGMENT };
    for (int i = 0; i < 2; i++) {
        shaders[i] = createShader(sources[i], stages[i], adapterInfo.shaderFormats);
        if (!shaders[i]) {
            DEBUG_BREAK();
            return false;
        }
    }

    PalPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {0};
    pipelineLayoutCreateInfo.descriptorSetLayoutCount = 1;
    pipelineLayoutCreateInfo.descriptorSetLayouts = &m_DescriptorSetLayout;
    pipelineLayoutCreateInfo.usePushConstant = PAL_TRUE;
    pipelineLayoutCreateInfo.pushConstantInfo.size = sizeof(PushConstant);

    result = palCreatePipelineLayout(
        device, 
        &pipelineLayoutCreateInfo, 
        &m_PipelineLayout);

    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create pipeline layout");
        DEBUG_BREAK();
        return false;
    }

    PalVertexAttribute attributes[3] = {
        { PAL_VERTEX_SEMANTIC_ID_POSITION, PAL_VERTEX_TYPE_FLOAT2 },
        { PAL_VERTEX_SEMANTIC_ID_TEXCOORD, PAL_VERTEX_TYPE_FLOAT2 },
        { PAL_VERTEX_SEMANTIC_ID_TEXCOORD, PAL_VERTEX_TYPE_UINT32 } // TEXCOORD1
    };

    PalVertexLayout vertexLayout = {0};
    vertexLayout.attributeCount = 3;
    vertexLayout.attributes = attributes;
    vertexLayout.binding = 0;
    vertexLayout.type = PAL_VERTEX_LAYOUT_TYPE_PER_VERTEX;

    PalRenderingLayoutInfo renderingLayoutInfo = {0};
    renderingLayoutInfo.colorAttachentCount = 1;
    renderingLayoutInfo.colorAttachmentsFormat = &imageInfo.format;
    renderingLayoutInfo.sampleCount = PAL_SAMPLE_COUNT_1;
    renderingLayoutInfo.viewCount = 1;

    // color blend attachment
    PalColorBlendAttachment blendAttachment = {0};
    blendAttachment.colorWriteMask |= PAL_COLOR_MASK_RED;
    blendAttachment.colorWriteMask |= PAL_COLOR_MASK_GREEN;
    blendAttachment.colorWriteMask |= PAL_COLOR_MASK_BLUE;
    blendAttachment.colorWriteMask |= PAL_COLOR_MASK_ALPHA;

    PalGraphicsPipelineCreateInfo pipelineCreateInfo = {0};
    pipelineCreateInfo.colorBlendAttachmentCount = 1;
    pipelineCreateInfo.colorBlendAttachments = &blendAttachment;
    pipelineCreateInfo.pipelineLayout = m_PipelineLayout;

    pipelineCreateInfo.renderingLayout = &renderingLayoutInfo;
    pipelineCreateInfo.topology = PAL_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineCreateInfo.vertexLayoutCount = 1;
    pipelineCreateInfo.vertexLayouts = &vertexLayout;

    pipelineCreateInfo.shaders = shaders;
    pipelineCreateInfo.shaderCount = 2;

    result = palCreateGraphicsPipeline(device, &pipelineCreateInfo, &m_QuadPipeline);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create graphics pipeline");
        DEBUG_BREAK();
        return false;
    }

    for (int i = 0; i < 2; i++) {
        palDestroyShader(shaders[i]);
    }

    // build indices
    uint32_t* indices = nullptr;
    indices = (uint32_t*)palAllocate(nullptr, sizeof(uint32_t) * MAX_INDICES, 0);
    if (!indices) {
        DEBUG_BREAK();
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

    PalBufferCreateInfo createInfo = {0};
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        Frame* frame = &m_Frames[i];

        createInfo.memoryUsage = PAL_BUFFER_MEMORY_USAGE_AUTO_GPU_ONLY;
        createInfo.usages = PAL_BUFFER_USAGE_VERTEX | PAL_BUFFER_USAGE_TRANSFER_DST;
        createInfo.size = sizeof(Vertex) * MAX_VERTICES;
        PalResult result = palCreateBuffer(device, &createInfo, &frame->vertexBuffer);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create buffer");
            DEBUG_BREAK();
            return false;
        }

        // create upload buffer
        createInfo.memoryUsage = PAL_BUFFER_MEMORY_USAGE_AUTO_CPU_UPLOAD;
        createInfo.usages = PAL_BUFFER_USAGE_TRANSFER_SRC;
        result = palCreateBuffer(device, &createInfo, &frame->uploadBuffer);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create buffer");
            DEBUG_BREAK();
            return false;
        }

        void* ptr = nullptr;
        result = palMapBuffer(frame->uploadBuffer, 0, createInfo.size, &ptr);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to map buffer");
            DEBUG_BREAK();
            return false;
        }

        frame->ptr = (Vertex*)ptr;
    }

    // create index buffer
    createInfo.memoryUsage = PAL_BUFFER_MEMORY_USAGE_AUTO_GPU_ONLY;
    createInfo.usages = PAL_BUFFER_USAGE_INDEX | PAL_BUFFER_USAGE_TRANSFER_DST;
    createInfo.size = sizeof(uint32_t) * MAX_INDICES;

    result = palCreateBuffer(device, &createInfo, &m_IndexBuffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create buffer");
        DEBUG_BREAK();
        return false;
    }

    // create staging buffer for index buffer
    createInfo.memoryUsage = PAL_BUFFER_MEMORY_USAGE_AUTO_CPU_UPLOAD;
    createInfo.usages = PAL_BUFFER_USAGE_TRANSFER_SRC;
    createInfo.size = sizeof(uint32_t) * MAX_INDICES;

    result = palCreateBuffer(device, &createInfo, &m_IndexStagingBuffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create buffer");
        DEBUG_BREAK();
        return false;
    }

    // copy indices to staging buffer
    void* ptr = nullptr;
    result = palMapBuffer(m_IndexStagingBuffer, 0, createInfo.size, &ptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to map buffer");
        DEBUG_BREAK();
        return false;
    }

    memcpy(ptr, indices, createInfo.size);
    palUnmapBuffer(m_IndexStagingBuffer);

    // copy staging buffer to index buffer
    result = palCmdBegin(m_Frames[0].cmdBuffer, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to begin command buffer");
        DEBUG_BREAK();
        return false;
    }

    PalBufferCopyInfo copyInfo = {0};
    copyInfo.size = createInfo.size;
    palCmdCopyBuffer(m_Frames[0].cmdBuffer, m_IndexBuffer, m_IndexStagingBuffer, &copyInfo);

    result = palCmdEnd(m_Frames[0].cmdBuffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to end command buffer");
        DEBUG_BREAK();
        return false;
    }

    result = palResetFence(m_Frames[0].fence);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to reset fence");
        DEBUG_BREAK();
        return false;
    }

    PalCommandBufferSubmitInfo submitInfo = {0};
    submitInfo.cmdBuffer = m_Frames[0].cmdBuffer;
    submitInfo.fence = m_Frames[0].fence;
    result = palSubmitCommandBuffer(m_Queue, &submitInfo);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to submit command buffer");
        DEBUG_BREAK();
        return false;
    }

    result = palWaitFence(m_Frames[0].fence, PAL_INFINITE);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to wait fence");
        DEBUG_BREAK();
        return false;
    }

    palDestroyBuffer(m_IndexStagingBuffer);
    result = palResetCommandBuffer(m_Frames[0].cmdBuffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to reset command buffer");
        DEBUG_BREAK();
        return false;
    }

    s_Vertices[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
    s_Vertices[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
    s_Vertices[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
    s_Vertices[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

    s_TextureCoords[0] = { 0.0f, 0.0f };
    s_TextureCoords[1] = { 1.0f, 0.0f };
    s_TextureCoords[2] = { 1.0f, 1.0f };
    s_TextureCoords[3] = { 0.0f, 1.0f };

    m_ImageRange.layerArrayCount = 1;
    m_ImageRange.mipLevelCount = 1;
    m_ImageRange.startArrayLayer = 0;
    m_ImageRange.startMipLevel = 0;

    m_Viewport.width = (float)windowWidth;
    m_Viewport.height = (float)windowHeight;
    m_Viewport.maxDepth = 1.0f;
    m_Scissor.width = windowWidth;
    m_Scissor.height = windowHeight;

    PalSamplerCreateInfo samplerCreateInfo = {0};
    samplerCreateInfo.addressModeU = PAL_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = PAL_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = PAL_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.borderColor = PAL_BORDER_COLOR_INT_OPAQUE_BLACK;

    samplerCreateInfo.compareOp = PAL_COMPARE_OP_NEVER;
    samplerCreateInfo.enableAnisotropy = PAL_FALSE;
    samplerCreateInfo.enableCompare = PAL_FALSE;
    samplerCreateInfo.magFilterMode = PAL_FILTER_MODE_LINEAR;

    samplerCreateInfo.maxAnisotropy = 1.0f;
    samplerCreateInfo.minFilterMode = PAL_FILTER_MODE_LINEAR;
    result = palCreateSampler(device, &samplerCreateInfo, &m_Sampler);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create sampler");
        DEBUG_BREAK();
        return false;
    }

    PalDescriptorSamplerInfo samplerInfo = {0};
    samplerInfo.sampler = m_Sampler;

    PalDescriptorSetWriteInfo writeInfo = {0};
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorSet = m_DescriptorSet;
    writeInfo.descriptorType = PAL_DESCRIPTOR_TYPE_SAMPLER;
    writeInfo.layoutBindingIndex = 1;

    writeInfo.samplerInfos = &samplerInfo;
    result = palUpdateDescriptorSet(device, 1, &writeInfo);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to update descriptor set");
        DEBUG_BREAK();
        return false;
    }

    palFree(nullptr, indices);
    return true;
}

void Renderer::shutdown()
{
    palWaitQueue(m_Queue);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        palDestroyBuffer(m_Frames[i].vertexBuffer);
        palUnmapBuffer(m_Frames[i].uploadBuffer);
        palDestroyBuffer(m_Frames[i].uploadBuffer);

        palDestroySemaphore(m_Frames[i].semaphore);
        palDestroyFence(m_Frames[i].fence);
        palFreeCommandBuffer(m_Frames[i].cmdBuffer);
    }

    for (int i = 0; i < m_ImageCount; i++) {
        palDestroySemaphore(m_RenderFinishedSemaphores[i]);
        palDestroyImageView(m_ImageViews[i]);
    }

    palDestroySampler(m_Sampler);
    palDestroyPipeline(m_QuadPipeline);
    palDestroyPipelineLayout(m_PipelineLayout);
    palDestroyDescriptorPool(m_DescriptorPool);
    palDestroyDescriptorSetLayout(m_DescriptorSetLayout);
    
    palDestroyCommandPool(m_CmdPool);
    palDestroyBuffer(m_IndexBuffer);

    palDestroySwapchain(m_Swapchain);
    palDestroySurface(m_Surface);
    palDestroyQueue(m_Queue);

    palFree(nullptr, m_ImageViews);
    palFree(nullptr, m_RenderFinishedSemaphores);
    palFree(nullptr, m_InFlightImages);
}

void Renderer::beginRendering(Camera* camera, const glm::vec4& clearColor)
{
    Frame* frame = &m_Frames[m_FrameIndex];

    PalResult result = palWaitFence(frame->fence, PAL_INFINITE);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to wait fence");
        DEBUG_BREAK();
        return;
    }

    PalSwapchainNextImageInfo nextImageInfo = {0};
    nextImageInfo.fence = nullptr;
    nextImageInfo.signalSemaphore = frame->semaphore;
    nextImageInfo.timeout = PAL_INFINITE;

    result = palGetNextSwapchainImage(m_Swapchain, &nextImageInfo, &m_ImageIndex);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to get next swapchain image");
        DEBUG_BREAK();
        return;
    }

    if (m_InFlightImages[m_ImageIndex] != nullptr) {
        result = palWaitFence(m_InFlightImages[m_ImageIndex], PAL_INFINITE);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to wait fence");
            DEBUG_BREAK();
            return;
        }
    }

    m_InFlightImages[m_ImageIndex] = frame->fence;
    result = palResetFence(frame->fence);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to wait fence");
        DEBUG_BREAK();
        return;
    }

    result = palResetCommandBuffer(frame->cmdBuffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to reset command buffer");
        DEBUG_BREAK();
        return;
    }

    result = palCmdBegin(frame->cmdBuffer, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to begin command buffer");
        DEBUG_BREAK();
        return;
    }

    PalImage* image = palGetSwapchainImage(m_Swapchain, m_ImageIndex);
    PalImageView* imageView = m_ImageViews[m_ImageIndex];

    PalBarrierInfo barrierInfo = {0};
    barrierInfo.newState = PAL_USAGE_STATE_COLOR_ATTACHMENT_WRITE;
    barrierInfo.dstStages = PAL_PIPELINE_STAGE_COLOR_ATTACHMENT;
    palCmdImageBarrier(frame->cmdBuffer, image, &m_ImageRange, &barrierInfo);

    PalClearValue clearValue;
    clearValue.color[0] = clearColor.r;
    clearValue.color[1] = clearColor.g;
    clearValue.color[2] = clearColor.b;
    clearValue.color[3] = clearColor.a;

    PalAttachmentDesc colorAttachment = {0};
    colorAttachment.loadOp = PAL_LOAD_OP_CLEAR;
    colorAttachment.storeOp = PAL_STORE_OP_STORE;
    colorAttachment.clearValue = clearValue;
    colorAttachment.imageView = imageView;

    PalRenderingInfo renderingInfo = {0};
    renderingInfo.viewCount = 1;
    renderingInfo.colorAttachentCount = 1;
    renderingInfo.colorAttachments = &colorAttachment;
    renderingInfo.arrayLayerCount = 1;
    renderingInfo.viewCount = 1;
    renderingInfo.renderArea.width = (uint32_t)m_Viewport.width;
    renderingInfo.renderArea.height = (uint32_t)m_Viewport.height;

    palCmdBeginRendering(frame->cmdBuffer, &renderingInfo);
    resetBatch();
    m_Projection = camera->getViewProjectionMatrix();
}

void Renderer::drawQuad(const glm::vec2& position, const glm::vec2& size, Texture* texture)
{
    nextBatch();
    uint32_t textureIndex = getTextureIndex(texture);
    Frame* frame = &m_Frames[m_FrameIndex];

    glm::mat4 transform = glm::mat4(1.0f);
    glm::translate(transform, { -position, 0.0f });
    glm::scale(transform, { size, 1.0f });
    
    for (int i = 0; i < 4; i++) {
        Vertex* quad = &frame->ptr[frame->offset + i];
        quad->pos = transform * s_Vertices[i];
        quad->texIndex = textureIndex;
        quad->uv = s_TextureCoords[i];
    }

    frame->offset += 4;
    frame->indexCount += 6;
    frame->vertexCount += 4;
}

void Renderer::endRendering()
{
    Frame* frame = &m_Frames[m_FrameIndex];
    PalSemaphore* m_RenderFinishedSemaphore = m_RenderFinishedSemaphores[m_ImageIndex];
    PalImage* image = palGetSwapchainImage(m_Swapchain, m_ImageIndex);

    flushBatch();
    palCmdEndRendering(frame->cmdBuffer);

    PalBarrierInfo barrierInfo = {0};
    barrierInfo.oldState = PAL_USAGE_STATE_COLOR_ATTACHMENT_WRITE;
    barrierInfo.srcStages = PAL_PIPELINE_STAGE_COLOR_ATTACHMENT;
    barrierInfo.newState = PAL_USAGE_STATE_PRESENT;
    barrierInfo.dstStages = PAL_PIPELINE_STAGE_NONE;

    palCmdImageBarrier(frame->cmdBuffer, image, &m_ImageRange, &barrierInfo);
    PalResult result = palCmdEnd(frame->cmdBuffer);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to end rendering");
        DEBUG_BREAK();
        return;
    }

    PalCommandBufferSubmitInfo submitInfo = {0};
    submitInfo.cmdBuffer = frame->cmdBuffer;
    submitInfo.fence = frame->fence;
    submitInfo.waitSemaphore = frame->semaphore;
    submitInfo.signalSemaphore = m_RenderFinishedSemaphore;
    submitInfo.waitStages = PAL_PIPELINE_STAGE_COLOR_ATTACHMENT;

    result = palSubmitCommandBuffer(m_Queue, &submitInfo);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to submit command buffer");
        DEBUG_BREAK();
        return;
    }

    result = palPresentSwapchain(m_Swapchain, m_ImageIndex, m_RenderFinishedSemaphore);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to present m_Swapchain");
        DEBUG_BREAK();
        return;
    }

    m_FrameIndex = (m_FrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::resize(uint32_t width, uint32_t height)
{
    float scaleX = (float)width / WORLD_WIDTH;
    float scaleY = (float)height / WORLD_HEIGHT;
    float scale = min(scaleX, scaleY);

    m_Viewport.width = WORLD_WIDTH * scale;
    m_Viewport.height = WORLD_HEIGHT * scale;
    m_Viewport.x = ((float)width - m_Viewport.width) * 0.5f;
    m_Viewport.y = ((float)height - m_Viewport.height) * 0.5f;

    m_Scissor.x = (uint32_t)m_Viewport.x;
    m_Scissor.y = (uint32_t)m_Viewport.y;
    m_Scissor.width = (uint32_t)m_Viewport.width;
    m_Scissor.height = (uint32_t)m_Viewport.height;
}

PalShader* Renderer::createShader(
    const char* path, 
    PalShaderStage stage,
    PalShaderFormats format)
{
    const char* mode = nullptr;
    void* buffer = nullptr;

    if ((format & PAL_SHADER_FORMAT_SPIRV) || 
       (format & PAL_SHADER_FORMAT_DXIL) ||
       (format & PAL_SHADER_FORMAT_DXBC)) {
        // binary formats
        mode = "rb";
    }

    FILE* file = fopen(path, "rb");
    if (!file) {
        return nullptr;
    }

    // find the size
    fseek(file, 0, SEEK_END);
    uint64_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = palAllocate(nullptr, size, 0);
    if (!buffer) {
        fclose(file);
    }

    // read contents
    size_t read = fread(buffer, 1, size, file);
    if (read != size) {
        return nullptr;
    }

    fclose(file);

    // create shader
    PalShaderEntryInfo entryInfo = {0};
    entryInfo.entryName = "main";
    entryInfo.stage = stage;

    PalShaderCreateInfo createInfo = {0};
    createInfo.code = buffer;
    createInfo.codeSize = (uint32_t)size;
    createInfo.entryCount = 1;
    createInfo.entries = &entryInfo;

    PalShader* shader = nullptr;
    PalResult result = palCreateShader(m_Device, &createInfo, &shader);
    if (result != PAL_RESULT_SUCCESS) {
        palFree(nullptr, buffer);
        logResult(result, "Failed to create shader");
        return nullptr;
    }

    palFree(nullptr, buffer);
    return shader;
}

void Renderer::resetBatch()
{
    Frame* frame = &m_Frames[m_FrameIndex];
    frame->offset = 0;
    frame->indexCount = 0;
    frame->vertexCount = 0;
    s_TextureSlotIndex = 0;
}

void Renderer::flushBatch()
{
    Frame* frame = &m_Frames[m_FrameIndex];
    if (frame->vertexCount) {
        uint32_t dataSize = sizeof(Vertex) * frame->vertexCount;

        PalBufferCopyInfo copyInfo = {0};
        copyInfo.size = dataSize;
        palCmdCopyBuffer(
            frame->cmdBuffer, 
            frame->vertexBuffer, 
            frame->uploadBuffer, 
            &copyInfo);

        PalBarrierInfo barrierInfo = {0};
        barrierInfo.oldState = PAL_USAGE_STATE_TRANSFER_WRITE;
        barrierInfo.srcStages = PAL_PIPELINE_STAGE_TRANSFER;
        barrierInfo.newState = PAL_USAGE_STATE_SHADER_READ;
        barrierInfo.dstStages = PAL_PIPELINE_STAGE_VERTEX_SHADER;

        PalDescriptorSetWriteInfo writeInfo = {0};
        writeInfo.descriptorCount = s_TextureSlotIndex;
        writeInfo.descriptorSet = m_DescriptorSet;
        writeInfo.descriptorType = PAL_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        writeInfo.layoutBindingIndex = 1;     
        
        writeInfo.imageViewInfos = s_TextureSlots;
        PalResult result = palUpdateDescriptorSet(m_Device, 1, &writeInfo);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to update descriptor set");
            DEBUG_BREAK();
            return;
        }

        palCmdBufferBarrier(frame->cmdBuffer, frame->vertexBuffer, &barrierInfo);
        palCmdBindPipeline(frame->cmdBuffer, m_QuadPipeline);
        palCmdSetViewport(frame->cmdBuffer, 1, &m_Viewport);
        palCmdSetScissors(frame->cmdBuffer, 1, &m_Scissor);

        PushConstant pushConstant;
        pushConstant.viewProjection = m_Projection;
        palCmdPushConstants(frame->cmdBuffer, 0, sizeof(PushConstant), &pushConstant);
        palCmdBindDescriptorSet(frame->cmdBuffer, 0, m_DescriptorSet);
        palCmdDrawIndexed(frame->cmdBuffer, frame->indexCount, 1, 0, 0, 0);
    }
}

void Renderer::nextBatch()
{
    Frame* frame = &m_Frames[m_FrameIndex];
    if (frame->vertexCount >= MAX_VERTICES || s_TextureSlotIndex >= MAX_TEXTURE_SLOTS) {
        flushBatch();
        resetBatch();
    }
}

uint32_t Renderer::getTextureIndex(Texture* texture)
{
    if (s_TextureSlotIndex == 0) {
        return 0;
    }

    for (uint32_t i = 0; i < s_TextureSlotIndex; i++) {
        if (s_TextureSlots[i].imageView == texture->imageView) {
            return i; 
        }
    }

    nextBatch();
    uint32_t index = s_TextureSlotIndex++;
    s_TextureSlots[index].imageView = texture->imageView;
    return index;
}