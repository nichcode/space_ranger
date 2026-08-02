
#include "game.h"

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
    createInfo.height = 480;
    createInfo.width = 640;
    createInfo.show = PAL_TRUE;
    createInfo.title = "Space Ranger";

    // check if we support decorated windows (title bar, close etc)
    PalVideoFeatures features = palGetVideoFeatures();
    if (!(features & PAL_VIDEO_FEATURE_DECORATED_WINDOW)) {
        // if we dont support, we need to create a borderless window
        // and create the decorations ourselves
        createInfo.style |= PAL_WINDOW_STYLE_BORDERLESS;
    }

    result = palCreateWindow(&createInfo, &m_Window);
    if (result != PAL_RESULT_SUCCESS) {
        logResult(result, "Failed to create window");
        return PAL_FALSE;
    }

    palSetEventDispatchMode(m_EventDriver, PAL_EVENT_TYPE_WINDOW_CLOSE, PAL_DISPATCH_MODE_POLL);
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
            }
        }
    }
}

void Game::shutdown()
{
    palDestroyWindow(m_Window);
    palShutdownVideo();
    palDestroyEventDriver(m_EventDriver);
}