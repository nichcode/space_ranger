
#include "game.h"

void Game::Init()
{
    b8 success = PAL_Init();
    if (!success) {
        std::cout << PAL_GetError() << std::endl;
        return;
    }

    u32 flags = PAL_WindowFlags_Center | PAL_WindowFlags_Show;
    m_Window = PAL_CreateWindow("Space Ranger", 640, 480, flags);
    if (!m_Window) {
        std::cout << PAL_GetError() << std::endl;
        return;
    }

    PAL_ContextDesc context_desc;
    context_desc.major = 3;
    context_desc.minor = 3;
    context_desc.type = PAL_ContextTypes_OpenGL;
    m_Context = PAL_CreateContext(m_Window, context_desc);
    if (!m_Context) {
        std::cout << PAL_GetError() << std::endl;
        return;
    }

    PAL_MakeActive(m_Context);
}

void Game::Shutdown()
{
    PAL_DestroyContext(m_Context);
    PAL_DestroyWindow(m_Window);
    PAL_Shutdown();
}

void Game::Run()
{
    while (!PAL_WindowShouldClose(m_Window)) {
        PAL_PullEvents();

        PAL_Clear({ .2f, .2f, .2f, 1.0f });
        PAL_SwapBuffers();
    }
}