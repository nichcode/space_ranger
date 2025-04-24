
#include "game.h"
#include "meteor.h"

#include <iostream>
#include <random>

void Game::Init()
{
    b8 success = PAL_Init();
    if (!success) {
        std::cout << PAL_GetError() << std::endl; // PAL must be initialized before using PAL_Log
        return;
    }

    u32 flags = PAL_WindowFlags_Center | PAL_WindowFlags_Show;
    m_Window = PAL_CreateWindow("Space Ranger", 640, 480, flags);
    if (!m_Window) {
        PAL_LogError(PAL_GetError());
        return;
    }

    PAL_ContextDesc context_desc;
    context_desc.major = 3;
    context_desc.minor = 3;
    context_desc.type = PAL_ContextTypes_OpenGL;
    m_Context = PAL_CreateContext(m_Window, context_desc);
    if (!m_Context) {
        PAL_LogError(PAL_GetError());
        return;
    }

    PAL_MakeActive(m_Context);
    PAL_SetVsync(true);

    m_Renderer = PAL_CreateRenderer();
    if (!m_Renderer) {
        PAL_LogError(PAL_GetError());
        return;
    }

    // load shared assets
    m_Font = PAL_LoadFont("assets/font.ttf", 30.0f);

    PAL_Camera camera;
    camera.view = { 0.0f, 0.0f, 640.0f, 480.0f };
    camera.rotation = 0.0f;
    PAL_SetRendererCamera(m_Renderer, camera);

    // states
    TitleState* title_state = new TitleState();
    GameState* game_state = new GameState();
    m_States.AddState("title", title_state);
    m_States.AddState("game", game_state);

    m_Data.window = m_Window;
    m_Data.font = m_Font;
    m_Data.renderer = m_Renderer;
}

void Game::Shutdown()
{
    m_States.Destroy();
    if (m_Renderer) {
        PAL_DestroyRenderer(m_Renderer);
    }
    if (m_Context) {
        PAL_DestroyContext(m_Context);
    }
    if (m_Window) {
        PAL_DestroyWindow(m_Window);
    }
    PAL_Shutdown();
}

void Game::Run()
{
    while (!PAL_WindowShouldClose(m_Window)) {
        PAL_PullEvents();
        m_States.Update(&m_Data);
        PAL_RendererFlush(m_Renderer);
        PAL_SwapBuffers();
    }
}