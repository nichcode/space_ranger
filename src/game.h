
#pragma once

#include "pal2/pal_video.h"
#include "helper.h"

struct Game
{
public:
    bool initialize();
    void shutdown();
    void run();

private:
    PalWindow* m_Window;
    PalEventDriver* m_EventDriver;

    bool m_Running;
};