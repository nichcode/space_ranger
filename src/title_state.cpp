
#include "title_state.h"
#include "PAL/PAL.h"

void TitleState::Update(PersistData* data)
{
    PAL_Clear({.2f, .2f, .2f, 1.0f});

    if (PAL_GetKeyState(data->window, PAL_Keys_Enter)) {
        SwitchState("game");
    }

    PAL_RendererDrawText(data->renderer, 250.0f, 200.0f, 1.0f, 
        "Title Screen", data->font, { 1.0f, 1.0f, 1.0f, 1.0f });
}