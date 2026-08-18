
#include "game.h"
#include "asset_manager.h"
#include "helper.h"

bool Game::initialize()
{
    PalEventDriverCreateInfo eventDriverCreateInfo = {0};
    PalResult result = palCreateEventDriver(&eventDriverCreateInfo, &m_EventDriver);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create event driver");
        DEBUG_BREAK();
        return false;
    }

    result = palInitGraphics(nullptr, nullptr, 0, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to initialize graphics");
        DEBUG_BREAK();
        return false;
    }

    // find an adapter and 

    result = palInitVideo(nullptr, m_EventDriver, nullptr);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to initialize video");
        DEBUG_BREAK();
        return false;
    }

    // create threads for renderer and assets manager

    PalWindowCreateInfo createInfo = {0};
    // createInfo.width = WINDOW_WIDTH;
    // createInfo.height = WINDOW_HEIGHT;
    createInfo.show = PAL_TRUE;
    createInfo.title = "Space Ranger";
    createInfo.style = PAL_WINDOW_STYLE_RESIZABLE;

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
        DEBUG_BREAK();
        return false;
    }

    palSetEventDispatchMode(
        m_EventDriver, 
        PAL_EVENT_TYPE_WINDOW_CLOSE, 
        PAL_DISPATCH_MODE_POLL);

    palSetEventDispatchMode(
        m_EventDriver, 
        PAL_EVENT_TYPE_WINDOW_SIZE, 
        PAL_DISPATCH_MODE_POLL);

    palSetEventDispatchMode(
        m_EventDriver, 
        PAL_EVENT_TYPE_KEYDOWN, 
        PAL_DISPATCH_MODE_POLL);

    palSetEventDispatchMode(
        m_EventDriver, 
        PAL_EVENT_TYPE_KEYREPEAT, 
        PAL_DISPATCH_MODE_POLL);

    palSetEventDispatchMode(
        m_EventDriver, 
        PAL_EVENT_TYPE_KEYUP, 
        PAL_DISPATCH_MODE_POLL);

    // m_Renderer.initialize(m_Window);
    // AssetManager::initialize(&m_Renderer);
    // m_Player.initialize();

    // m_Camera.setPosition({ 0.0f, 0.0f });
    // m_Camera.setSize({ WORLD_WIDTH, WORLD_HEIGHT });
    // m_Camera.setRotation(0.0f);

    // m_Running = true;
    return true;
}

void Game::run()
{
    while (m_Running) {
        palUpdateVideo();

        PalEvent event;
        while (palPollEvent(m_EventDriver, &event)) {
            switch (event.type) {
                case PAL_EVENT_TYPE_WINDOW_CLOSE: {
                    m_Running = PAL_FALSE;
                    break;
                }

                case PAL_EVENT_TYPE_WINDOW_SIZE: {
                    uint32_t width, height;
                    palUnpackUint32(event.data, &width, &height);
                    // m_Renderer.resize(width, height);
                }
            }
        }

        // m_Renderer.beginRendering(&m_Camera, { 0.0f, 0.0f, 0.0f, 1.0f });
        // m_Player.render(&m_Renderer);
        // m_Renderer.endRendering();
    }
}

void Game::shutdown()
{
    // m_Player.destroy();
    // AssetManager::shutdown();
    // m_Renderer.shutdown();
    palDestroyWindow(m_Window);
    palShutdownVideo();
    palDestroyEventDriver(m_EventDriver);
    palShutdownGraphics();
}

void Game::createDevice()
{

}