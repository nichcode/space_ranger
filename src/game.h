
#pragma once

#include "PAL/PAL.h"
#include "state_machine.h"

// states
#include "title_state.h"
#include "game_state.h"

class Game
{
private:
    PAL_Window* m_Window = nullptr;
    PAL_Context* m_Context = nullptr;
    PAL_Renderer* m_Renderer = nullptr;

    PAL_Font* m_Font = nullptr;

    PersistData m_Data;
    StateMachine m_States;

public:
    void Init();
    void Shutdown();
    void Run();
};

i32 GetRandomNum();