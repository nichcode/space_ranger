
#pragma once

#include "pal2/pal_video.h"
#include "pal2/pal_graphics.h"

#define MAX_FRAMES_IN_FLIGHT 2

struct Game {
public:
    bool initialize();
    void shutdown();
    void run();

private:
bool m_Running;
    PalWindow* m_Window;
    PalEventDriver* m_EventDriver;
    PalAdapter* m_Adapter;
    PalDevice* m_Device;
    PalQueue* m_Queue;
    PalSurface* m_Surface;
    PalSwapchain* m_Swapchain;

    PalCommandPool* m_CmdPool = nullptr;
    PalImageView** m_ImageViews = nullptr;
    PalCommandBuffer* m_CmdBuffers[MAX_FRAMES_IN_FLIGHT];
    PalSemaphore* m_ImageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];

    uint32_t m_ImageCount;
    PalFence* m_InFlightFences[MAX_FRAMES_IN_FLIGHT];
    PalSemaphore** m_RenderFinishedSemaphores;
    PalFence** m_InFlightImages;
};