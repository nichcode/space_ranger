
#pragma once

#include "pal2/pal_video.h"
#include "renderer.h"
#include "camera.h"
#include "player.h"

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

    Camera m_Camera;
    Player m_Player;
};