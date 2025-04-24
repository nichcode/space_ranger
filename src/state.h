
#pragma once

#include "PAL/PAL_window.h"
#include "PAL/PAL_renderer.h"
#include "state_machine.h"

struct PersistData
{
    PAL_Window* window;
    PAL_Renderer* renderer;
    PAL_Font* font;
};

class State
{
private:
    StateMachine* m_Machine;
    friend class StateMachine;

public:
    virtual ~State() {}
    virtual void Update(PersistData* data) {}

    void SwitchState(const char* state_name)
    {
        m_Machine->MakeActive(state_name);
    }
};