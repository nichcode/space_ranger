
#pragma once

#include "PAL/PAL.h"
#include <iostream>

class Game
{
private:
    PAL_Window* m_Window;
    PAL_Context* m_Context;

public:
    void Init();
    void Shutdown();
    void Run();
};