
#pragma once

#include "pal2/pal_video.h"
#include "renderer.h"

class Game {
public:
    bool initialize();
    void shutdown();
    void run();

private:
    bool m_Running;
    PalWindow* m_Window;
    PalEventDriver* m_EventDriver;
    Renderer m_Renderer;
};