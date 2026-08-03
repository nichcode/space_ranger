
#include "game.h"
#include "pal2/pal_system.h"
#include "helper.h"

bool Game::initialize()
{
    PalEventDriverCreateInfo eventDriverCreateInfo = {0};
    PalResult result = palCreateEventDriver(&eventDriverCreateInfo, &m_EventDriver);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create event driver");
        return false;
    }

    result = palInitVideo(nullptr, m_EventDriver, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to initialize video");
        return false;
    }

    PalWindowCreateInfo createInfo = {0};
    createInfo.width = WINDOW_WIDTH;
    createInfo.height = WINDOW_HEIGHT;
    createInfo.show = PAL_TRUE;
    createInfo.title = "Space Ranger";

    // check if we support decorated windows (title bar, close etc)
    PalVideoFeatures videoFeatures = palGetVideoFeatures();
    if (!(videoFeatures & PAL_VIDEO_FEATURE_DECORATED_WINDOW)) {
        // if we dont support, we need to create a borderless window
        // and create the decorations ourselves
        createInfo.style |= PAL_WINDOW_STYLE_BORDERLESS;
    }

    result = palCreateWindow(&createInfo, &m_Window);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create window");
        return false;
    }

    PalPlatformInfo platformInfo = {0};
    PalWindowHandleInfo winHandle = {0};
    palGetWindowHandleInfo(m_Window, &winHandle);
    palGetPlatformInfo(&platformInfo);

    PalWindowInstanceType windowInstanceType = PAL_WINDOW_INSTANCE_TYPE_XCB;
    if (platformInfo.apiType == PAL_PLATFORM_API_TYPE_WAYLAND) {
        windowInstanceType = PAL_WINDOW_INSTANCE_TYPE_WAYLAND;

    } else if (platformInfo.apiType == PAL_PLATFORM_API_TYPE_X11) {
        windowInstanceType = PAL_WINDOW_INSTANCE_TYPE_X11;

    } else if (platformInfo.apiType == PAL_PLATFORM_API_TYPE_WIN32) {
        windowInstanceType = PAL_WINDOW_INSTANCE_TYPE_WIN32;
    }

    palSetEventDispatchMode(m_EventDriver, PAL_EVENT_TYPE_WINDOW_CLOSE, PAL_DISPATCH_MODE_POLL);

    result = palInitGraphics(nullptr, nullptr, 0, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to initialize graphics");
        return false;
    }

    // enumerate all available adapters
    uint32_t adapterCount = 0;
    result = palEnumerateAdapters(&adapterCount, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to get adapters");
        return false;
    }

    if (adapterCount == 0) {
        palLog(nullptr, "No adapters found");
        return false;
    }

    PalAdapter** adapters = nullptr;
    adapters = (PalAdapter**)palAllocate(nullptr, sizeof(PalAdapter*) * adapterCount, 0);
    if (!adapters) {
        palLog(nullptr, "Failed to allocate memory");
        return false;
    }

    result = palEnumerateAdapters(&adapterCount, adapters);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to get adapters");
        return false;
    }

    PalAdapterCapabilities caps = {0};
    PalAdapterFeatures adapterFeatures = 0;
    PalAdapterInfo adapterInfo = {0};
    for (int32_t i = 0; i < adapterCount; i++) {
        m_Adapter = adapters[i];
        palGetAdapterInfo(m_Adapter, &adapterInfo);
        if (adapterInfo.type == PAL_ADAPTER_TYPE_CPU) {
            continue;
        }

        palGetAdapterCapabilities(m_Adapter, &caps);
        if (caps.maxGraphicsQueues == 0) {
            m_Adapter = nullptr;
            continue;

        } else {
            adapterFeatures = palGetAdapterFeatures(m_Adapter);
            if (!(adapterFeatures & PAL_ADAPTER_FEATURE_SWAPCHAIN)) {
                continue;
            }

            if (!(adapterFeatures & PAL_ADAPTER_FEATURE_FENCE_RESET)) {
                continue;
            }
            break;
        }
    }

    palFree(nullptr, adapters);
    if (!m_Adapter) {
        palLog(nullptr, "Failed to find a required adapter");
        return PAL_FALSE;
    }

    // create device
    PalAdapterFeatures features = PAL_ADAPTER_FEATURE_SWAPCHAIN;
    result = palCreateDevice(m_Adapter, features, &m_Device);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create m_Device");
        return false;
    }

    // create m_Surface
    result = palCreateSurface(
        m_Device,
        winHandle.nativeWindow,
        winHandle.nativeInstance,
        windowInstanceType,
        &m_Surface);

    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create m_Surface");
        return false;
    }

    // create a graphics command m_Queue and check if its supports presenting to the m_Surface
    PalBool foundQueue = PAL_FALSE;
    for (int i = 0; i < caps.maxGraphicsQueues; i++) {
        result = palCreateQueue(m_Device, PAL_QUEUE_TYPE_GRAPHICS, &m_Queue);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create m_Queue");
            return false;
        }

        if (!palCanQueuePresent(m_Queue, m_Surface)) {
            palDestroyQueue(m_Queue);
        } else {
            // found a m_Queue
            foundQueue = PAL_TRUE;
            break;
        }
    }

    if (!foundQueue) {
        palLog(nullptr, "Failed to find a m_Queue that can present to the m_Surface");
        return false;
    }

    // create a m_Swapchain with the graphics m_Queue
    PalSurfaceCapabilities surfaceCaps = {0};
    palGetSurfaceCapabilities(m_Device, m_Surface, &surfaceCaps);

    PalSwapchainCreateInfo swapchainCreateInfo = {0};
    swapchainCreateInfo.clipped = PAL_TRUE;
    swapchainCreateInfo.compositeAlpha = PAL_COMPOSITE_ALPHA_OPAQUE;
    swapchainCreateInfo.height = WINDOW_HEIGHT;
    swapchainCreateInfo.width = WINDOW_WIDTH;
    swapchainCreateInfo.imageArrayLayerCount = 1;
    swapchainCreateInfo.presentMode = PAL_PRESENT_MODE_FIFO;
    swapchainCreateInfo.format = PAL_SURFACE_FORMAT_BGRA8_UNORM_SRGB_NONLINEAR;

    // rare but possible on andriod
    if (WINDOW_WIDTH > surfaceCaps.maxImageWidth) {
        swapchainCreateInfo.width = surfaceCaps.maxImageWidth / 2;
    }

    if (WINDOW_HEIGHT > surfaceCaps.maxImageHeight) {
        swapchainCreateInfo.height = surfaceCaps.maxImageHeight / 2;
    }

    // check if the minimal image count is not good for you
    // and increase it but not pass the max count
    swapchainCreateInfo.imageCount = surfaceCaps.minImageCount;
    if (swapchainCreateInfo.imageCount == 1) {
        swapchainCreateInfo.imageCount++;
        if (surfaceCaps.maxImageCount < 2) {
            palLog(nullptr, "Surface does not support double buffers");
            return false;
        }
    }

    result = palCreateSwapchain(m_Device, m_Queue, m_Surface, &swapchainCreateInfo, &m_Swapchain);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create m_Swapchain");
        return false;
    }

    // get all m_Swapchain images and create image views for them
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
            palLog(nullptr, "Failed to get m_Swapchain image");
        }

        result = palCreateImageView(m_Device, image, &imageViewCreateInfo, &m_ImageViews[i]);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create image view");
            return false;
        }

        // create render finished semaphores
        result = palCreateSemaphore(m_Device, PAL_FALSE, &m_RenderFinishedSemaphores[i]);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create semaphore");
            return false;
        }

        m_InFlightImages[i] = nullptr;
    }

    result = palCreateCommandPool(m_Device, m_Queue, &m_CmdPool);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create command pool");
        return false;
    }

    // create synchronization objects and command buffers
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        result = palCreateSemaphore(m_Device, PAL_FALSE, &m_ImageAvailableSemaphores[i]);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create semaphore");
            return false;
        }

        result = palCreateFence(m_Device, PAL_TRUE, &m_InFlightFences[i]);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to create fence");
            return false;
        }

        result = palAllocateCommandBuffer(
            m_Device,
            m_CmdPool,
            PAL_COMMAND_BUFFER_TYPE_PRIMARY,
            &m_CmdBuffers[i]);

        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to allocate command buffer");
            return false;
        }
    }

    m_Running = true;
    return true;
}

void Game::run()
{
    uint32_t currentFrame = 0;
    while (m_Running) {
        palUpdateVideo();

        PalEvent event;
        while (palPollEvent(m_EventDriver, &event)) {
            switch (event.type) {
                case PAL_EVENT_TYPE_WINDOW_CLOSE: {
                    m_Running = PAL_FALSE;
                    break;
                }
            }
        }

        PalResult result = palWaitFence(m_InFlightFences[currentFrame], PAL_INFINITE);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to wait fence");
            return;
        }

        // get next m_Swapchain image
        PalSwapchainNextImageInfo nextImageInfo = {0};
        nextImageInfo.fence = nullptr;
        nextImageInfo.signalSemaphore = m_ImageAvailableSemaphores[currentFrame];
        nextImageInfo.timeout = PAL_INFINITE;

        uint32_t imageIndex = 0;
        result = palGetNextSwapchainImage(m_Swapchain, &nextImageInfo, &imageIndex);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to get next m_Swapchain image");
            return;
        }

        if (m_InFlightImages[imageIndex] != nullptr) {
            result = palWaitFence(m_InFlightImages[imageIndex], PAL_INFINITE);
            if (result != PAL_RESULT_SUCCESS) {
                logResult(result, "Failed to wait fence");
                return;
            }
        }

        m_InFlightImages[imageIndex] = m_InFlightFences[currentFrame];
        result = palResetFence(m_InFlightFences[currentFrame]);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to wait fence");
            return;
        }

        // reset the command buffer
        result = palResetCommandBuffer(m_CmdBuffers[currentFrame]);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to reset command buffer");
            return;
        }

        result = palCmdBegin(m_CmdBuffers[currentFrame], nullptr);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to begin command buffer");
            return;
        }

        // change the state of the image view to make it renderable
        PalBarrierInfo barrierInfo = {0};
        barrierInfo.newState = PAL_USAGE_STATE_COLOR_ATTACHMENT_WRITE;
        barrierInfo.dstStages = PAL_PIPELINE_STAGE_COLOR_ATTACHMENT;

        PalImageSubresourceRange imageRange = {0};
        imageRange.layerArrayCount = 1;
        imageRange.mipLevelCount = 1;
        imageRange.startArrayLayer = 0;
        imageRange.startMipLevel = 0;

        PalImage* image = palGetSwapchainImage(m_Swapchain, imageIndex);
        palCmdImageBarrier(m_CmdBuffers[currentFrame], image, &imageRange, &barrierInfo);

        PalClearValue clearValue;
        clearValue.color[0] = 0.2f;
        clearValue.color[1] = 0.2f;
        clearValue.color[2] = 0.2f;
        clearValue.color[3] = 1.0f;

        PalAttachmentDesc colorAttachment = {0};
        colorAttachment.loadOp = PAL_LOAD_OP_CLEAR;
        colorAttachment.storeOp = PAL_STORE_OP_STORE;
        colorAttachment.clearValue = clearValue;
        colorAttachment.imageView = m_ImageViews[imageIndex];

        PalRenderingInfo renderingInfo = {0};
        renderingInfo.viewCount = 1;
        renderingInfo.colorAttachentCount = 1;
        renderingInfo.colorAttachments = &colorAttachment;
        renderingInfo.arrayLayerCount = 1;
        renderingInfo.viewCount = 1;
        renderingInfo.renderArea.width = WINDOW_WIDTH;
        renderingInfo.renderArea.height = WINDOW_HEIGHT;

        palCmdBeginRendering(m_CmdBuffers[currentFrame], &renderingInfo);
        palCmdEndRendering(m_CmdBuffers[currentFrame]);

        // change the state of the image view to make it presentable
        barrierInfo.oldState = barrierInfo.newState;
        barrierInfo.srcStages = barrierInfo.dstStages;
        barrierInfo.newState = PAL_USAGE_STATE_PRESENT;
        barrierInfo.dstStages = PAL_PIPELINE_STAGE_NONE;
        // palCmdImageBarrier(m_CmdBuffers[currentFrame], image, &imageRange, &barrierInfo);

        result = palCmdEnd(m_CmdBuffers[currentFrame]);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to end command buffer");
            return;
        }

        // submit command buffer
        PalCommandBufferSubmitInfo submitInfo = {0};
        submitInfo.cmdBuffer = m_CmdBuffers[currentFrame];
        submitInfo.fence = m_InFlightFences[currentFrame];
        submitInfo.waitSemaphore = m_ImageAvailableSemaphores[currentFrame];
        submitInfo.signalSemaphore = m_RenderFinishedSemaphores[imageIndex];
        submitInfo.waitStages = PAL_PIPELINE_STAGE_COLOR_ATTACHMENT;

        result = palSubmitCommandBuffer(m_Queue, &submitInfo);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to submit command buffer");
            return;
        }

        // present
        result = palPresentSwapchain(m_Swapchain, imageIndex, m_RenderFinishedSemaphores[imageIndex]);
        if (result != PAL_RESULT_SUCCESS) {
            logResult(result, "Failed to present m_Swapchain");
            return;
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}

void Game::shutdown()
{
    palWaitQueue(m_Queue);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        palDestroySemaphore(m_ImageAvailableSemaphores[i]);
        palDestroyFence(m_InFlightFences[i]);
        palFreeCommandBuffer(m_CmdBuffers[i]);
    }

    for (int i = 0; i < m_ImageCount; i++) {
        palDestroySemaphore(m_RenderFinishedSemaphores[i]);
        palDestroyImageView(m_ImageViews[i]);
    }

    palDestroyCommandPool(m_CmdPool);
    palDestroySwapchain(m_Swapchain);
    palDestroySurface(m_Surface);
    palDestroyQueue(m_Queue);
    palDestroyDevice(m_Device);
    palShutdownGraphics();

    palFree(nullptr, m_ImageViews);
    palFree(nullptr, m_RenderFinishedSemaphores);
    palFree(nullptr, m_InFlightImages);

    palDestroyWindow(m_Window);
    palShutdownVideo();
    palDestroyEventDriver(m_EventDriver);
}