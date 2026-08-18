
#include "pal2/pal_thread.h"
#include "pal2/pal_graphics.h"
#include "pal2/pal_video.h"
#include "pal2/pal_system.h"
#include "helper.h"

typedef struct {
    PalDevice* device;
    PalAdapterCapabilities adapterCaps;
    PalAdapterInfo adapterInfo;
    PalWindow* window;
} ThreadArgument;

static ThreadArgument s_ThreadArg = {0};

static bool createDevice(
    PalAdapterCapabilities* adapterCaps, 
    PalAdapterInfo* adapterInfo, 
    PalAdapter** outAdapter, 
    PalDevice** outDevice)
{
    uint32_t count = 0;
    PalAdapter* adapter = nullptr;
    PalDevice* device = nullptr;
    PalAdapterFeatures adapterFeatures = 0;

    PalResult result = palEnumerateAdapters(&count, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to get adapters");
        DEBUG_BREAK();
        return false;
    }

    if (count == 0) {
        palLog(nullptr, "No adapters found");
        DEBUG_BREAK();
        return false;
    }

    PalAdapter** adapters = nullptr;
    adapters = (PalAdapter**)palAllocate(nullptr, sizeof(PalAdapter*) * count, 0);
    if (!adapters) {
        palLog(nullptr, "Failed to allocate memory");
        DEBUG_BREAK();
        return false;
    }

    result = palEnumerateAdapters(&count, adapters);
    if (result != PAL_RESULT_SUCCESS) {
        palFree(nullptr, adapters);
        logResult(result, "Failed to get adapters");
        DEBUG_BREAK();
        return false;
    }

    for (int32_t i = 0; i < count; i++) {
        adapter = adapters[i];
        palGetAdapterInfo(adapter, adapterInfo);
        if (adapterInfo->type == PAL_ADAPTER_TYPE_CPU) {
            adapter = nullptr;
            continue;
        }

        if (adapterInfo->vtableVersion < PAL_GRAPHICS_BACKEND_VTABLE_VERSION_2) {
            adapter = nullptr;
            continue;
        }

        palGetAdapterCapabilities(adapter, adapterCaps);
        if (adapterCaps->maxGraphicsQueues == 0 || adapterCaps->maxCopyQueues == 0) {
            adapter = nullptr;
            continue;

        } else {
            adapterFeatures = palGetAdapterFeatures(adapter);
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
    if (!adapter) {
        palLog(nullptr, "Failed to find a required adapter");
        DEBUG_BREAK();
        return false;
    }

    // create device
    PalAdapterFeatures features = PAL_ADAPTER_FEATURE_SWAPCHAIN;
    features |= PAL_ADAPTER_FEATURE_FENCE_RESET;

    result = palCreateDevice(adapter, features, &device);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create device");
        DEBUG_BREAK();
        return false;
    }

    *outAdapter = adapter;
    *outDevice = device;
    return true;
}

// static bool createSwapchain(
//     PalDevice* device, 
//     PalWindow* window, 
//     PalSurface** outSurface, 
//     PalSwapchain** outSwapchain)
// {
//     // PalPlatformInfo platformInfo = {0};
//     // PalWindowHandleInfo winHandle = {0};
//     // palGetWindowHandleInfo(window, &winHandle);
//     // palGetPlatformInfo(&platformInfo);

//     // PalWindowInstanceType windowInstanceType = PAL_WINDOW_INSTANCE_TYPE_XCB;
//     // if (platformInfo.apiType == PAL_PLATFORM_API_TYPE_WAYLAND) {
//     //     windowInstanceType = PAL_WINDOW_INSTANCE_TYPE_WAYLAND;

//     // } else if (platformInfo.apiType == PAL_PLATFORM_API_TYPE_X11) {
//     //     windowInstanceType = PAL_WINDOW_INSTANCE_TYPE_X11;

//     // } else if (platformInfo.apiType == PAL_PLATFORM_API_TYPE_WIN32) {
//     //     windowInstanceType = PAL_WINDOW_INSTANCE_TYPE_WIN32;
//     // }

//     // PalSurface* surface = nullptr;
//     // PalResult result = palCreateSurface(
//     //     device,
//     //     winHandle.nativeWindow,
//     //     winHandle.nativeInstance,
//     //     windowInstanceType,
//     //     &surface);

//     // if (result != PAL_RESULT_SUCCESS) {
//     //     logResult(result, "Failed to create surface");
//     //     DEBUG_BREAK();
//     //     return false;
//     // }
// }

static void* PAL_CALL rendererInitWorker(void* arg)
{
    ThreadArgument* data = (ThreadArgument*)arg;
    palLog(nullptr, "Renderer Init Thread");
    return nullptr;
}

static void* PAL_CALL assetManagerInitWorker(void* arg)
{
    ThreadArgument* data = (ThreadArgument*)arg;
    palLog(nullptr, "Asset Manager Init Thread");
    return nullptr;
}

int main(int argc, char** argv)
{
    PalAdapter* adapter = nullptr;
    PalDevice* device = nullptr;
    PalWindow* window = nullptr;
    PalAdapterCapabilities adapterCaps;

    PalAdapterInfo adapterInfo;
    PalEventDriver* eventDriver = nullptr;

    PalResult result = palInitGraphics(nullptr, nullptr, 0, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to initialize graphics");
        DEBUG_BREAK();
        return -1;
    }

    PalEventDriverCreateInfo eventDriverCreateInfo = {0};
    result = palCreateEventDriver(&eventDriverCreateInfo, &eventDriver);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create event driver");
        DEBUG_BREAK();
        return -1;
    }

    result = palInitVideo(nullptr, eventDriver, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to initialize video");
        DEBUG_BREAK();
        return -1;
    }

    if (!createDevice(&adapterCaps, &adapterInfo, &adapter, &device)) {
        palLog(nullptr, "Failed to create device");
        DEBUG_BREAK();
        return -1;
    }

    s_ThreadArg.adapterCaps = adapterCaps;
    s_ThreadArg.adapterInfo = adapterInfo;
    s_ThreadArg.device = device;

    // create a thread to initialize the asset manager
    PalThreadCreateInfo threadCreateInfo = {0};
    threadCreateInfo.allocator = nullptr;
    threadCreateInfo.arg = (void*)&s_ThreadArg;
    threadCreateInfo.entry = assetManagerInitWorker;

    PalThread* assetManagerInitThread = nullptr;
    result = palCreateThread(&threadCreateInfo, &assetManagerInitThread);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create thread");
        DEBUG_BREAK();
        return -1;
    }

    // create window
    PalWindowCreateInfo windowCreateInfo = {0};
    windowCreateInfo.width = WINDOW_WIDTH;
    windowCreateInfo.height = WINDOW_HEIGHT;
    windowCreateInfo.show = PAL_TRUE;
    windowCreateInfo.title = "Space Ranger";
    windowCreateInfo.style = PAL_WINDOW_STYLE_RESIZABLE;

    // check if we support decorated windows (title bar, close etc)
    PalVideoFeatures videoFeatures = palGetVideoFeatures();
    if (!(videoFeatures & PAL_VIDEO_FEATURE_DECORATED_WINDOW)) {
        // if we dont support, we need to create a borderless window
        // and create the decorations ourselves
        windowCreateInfo.style |= PAL_WINDOW_STYLE_BORDERLESS;
    }

    result = palCreateWindow(&windowCreateInfo, &window);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create window");
        DEBUG_BREAK();
        return -1;
    }

    palSetEventDispatchMode(
        eventDriver, 
        PAL_EVENT_TYPE_WINDOW_CLOSE, 
        PAL_DISPATCH_MODE_POLL);

    palSetEventDispatchMode(
        eventDriver, 
        PAL_EVENT_TYPE_WINDOW_SIZE, 
        PAL_DISPATCH_MODE_POLL);

    palSetEventDispatchMode(
        eventDriver, 
        PAL_EVENT_TYPE_KEYDOWN, 
        PAL_DISPATCH_MODE_POLL);

    palSetEventDispatchMode(
        eventDriver, 
        PAL_EVENT_TYPE_KEYREPEAT, 
        PAL_DISPATCH_MODE_POLL);

    palSetEventDispatchMode(
        eventDriver, 
        PAL_EVENT_TYPE_KEYUP, 
        PAL_DISPATCH_MODE_POLL);

    // create a thread to initialize the renderer
    s_ThreadArg.window = window;
    threadCreateInfo.allocator = nullptr;
    threadCreateInfo.arg = (void*)&s_ThreadArg;
    threadCreateInfo.entry = rendererInitWorker;

    PalThread* rendererInitThread = nullptr;
    result = palCreateThread(&threadCreateInfo, &rendererInitThread);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create thread");
        DEBUG_BREAK();
        return -1;
    }

    // wait for the renderer and asset manager init thread
    result = palJoinThread(assetManagerInitThread, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to join thread");
        return -1;
    }

    result = palJoinThread(rendererInitThread, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to join thread");
        return -1;
    }

    palDestroyWindow(window);
    palShutdownVideo();
    palDestroyEventDriver(eventDriver);
    palShutdownGraphics();
}