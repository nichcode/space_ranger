
#include "pal2/pal_thread.h"
#include "pal2/pal_graphics.h"
#include "pal2/pal_video.h"
#include "asset_manager.h"

#include "renderer.h"
#include "helper.h"

struct GameData {
    AssetManager assetManager;
    Renderer renderer;
    PalDevice* device = nullptr;
    PalWindow* window = nullptr;
    PalAdapter* adapter = nullptr;
    uint32_t windowWidth = 0;
    uint32_t windowHeight = 0;
};

static GameData s_GameData;

static bool createDevice()
{
    uint32_t count = 0;
    PalAdapterFeatures adapterFeatures = 0;
    PalAdapterCapabilities adapterCaps = {};
    PalAdapterInfo adapterInfo = {};

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
        s_GameData.adapter = adapters[i];
        palGetAdapterInfo(s_GameData.adapter, &adapterInfo);
        if (adapterInfo.type == PAL_ADAPTER_TYPE_CPU) {
            s_GameData.adapter = nullptr;
            continue;
        }

        if (adapterInfo.vtableVersion < PAL_GRAPHICS_BACKEND_VTABLE_VERSION_2) {
            s_GameData.adapter = nullptr;
            continue;
        }

        palGetAdapterCapabilities(s_GameData.adapter, &adapterCaps);
        if (adapterCaps.maxGraphicsQueues == 0 || adapterCaps.maxCopyQueues == 0) {
            s_GameData.adapter = nullptr;
            continue;

        } else {
            adapterFeatures = palGetAdapterFeatures(s_GameData.adapter);
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
    if (!s_GameData.adapter) {
        palLog(nullptr, "Failed to find a required adapter");
        DEBUG_BREAK();
        return false;
    }

    // create device
    PalAdapterFeatures features = PAL_ADAPTER_FEATURE_SWAPCHAIN;
    features |= PAL_ADAPTER_FEATURE_FENCE_RESET;

    result = palCreateDevice(s_GameData.adapter, features, &s_GameData.device);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create device");
        DEBUG_BREAK();
        return false;
    }

    return true;
}

static void* PAL_CALL rendererInitWorker(void* arg)
{
    bool result = s_GameData.renderer.initialize(
        s_GameData.window, 
        s_GameData.adapter, 
        s_GameData.device, 
        s_GameData.windowWidth, 
        s_GameData.windowHeight);

    if (!result) {
        palLog(nullptr, "Failed to initialize renderer");
        DEBUG_BREAK();
    }
    return nullptr;
}

static void* PAL_CALL assetManagerInitWorker(void* arg)
{
    bool result = s_GameData.assetManager.initialize(s_GameData.device);
    if (!result) {
        palLog(nullptr, "Failed to initialize asset manager");
        DEBUG_BREAK();
    }
    return nullptr;
}

int main(int argc, char** argv)
{
    PalResult result = palInitGraphics(nullptr, nullptr, 0, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to initialize graphics");
        DEBUG_BREAK();
        return -1;
    }

    PalEventDriver* eventDriver = nullptr;
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

    if (!createDevice()) {
        palLog(nullptr, "Failed to create device");
        DEBUG_BREAK();
        return -1;
    }

    // create a thread to initialize the asset manager
    PalThreadCreateInfo threadCreateInfo = {0};
    threadCreateInfo.entry = assetManagerInitWorker;

    PalThread* assetManagerInitThread = nullptr;
    result = palCreateThread(&threadCreateInfo, &assetManagerInitThread);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create thread");
        DEBUG_BREAK();
        return -1;
    }

    // create window
    // TODO: get the first or primary monitor and create a borderless fullscreen
    // window
    uint32_t windowWidth = 640;
    uint32_t windowHeight = 480;
    s_GameData.windowWidth = windowWidth;
    s_GameData.windowHeight = windowHeight;

    PalWindowCreateInfo windowCreateInfo = {0};
    windowCreateInfo.width = windowWidth;
    windowCreateInfo.height = windowHeight;
    windowCreateInfo.show = PAL_TRUE;
    windowCreateInfo.title = "Space Ranger";
    windowCreateInfo.style = PAL_WINDOW_STYLE_RESIZABLE | PAL_WINDOW_STYLE_BORDERLESS;

    result = palCreateWindow(&windowCreateInfo, &s_GameData.window);
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
    PalThread* rendererInitThread = nullptr;
    threadCreateInfo.entry = rendererInitWorker;

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

    palDestroyWindow(s_GameData.window);
    palShutdownVideo();
    palDestroyEventDriver(eventDriver);
    s_GameData.assetManager.shutdown();

    s_GameData.renderer.shutdown();
    palDestroyDevice(s_GameData.device);
    palShutdownGraphics();
}