
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

    m_BulletTexture = PAL_LoadTexture("assets/bullet.png");
    m_MeteorTexture = PAL_LoadTexture("assets/meteor.png");

    PAL_Camera camera;
    camera.view = { 0.0f, 0.0f, 640.0f, 480.0f };
    camera.rotation = 0.0f;
    PAL_SetRendererCamera(m_Renderer, camera);

    m_Player.Init(m_Renderer, m_Window);

    // create meteors
    i32 count = 0;
    while (count < 30) {
        Meteor* meteor = new Meteor();
        meteor->Init(m_Renderer, m_MeteorTexture);
        m_Meteors.Add(meteor);
        count++;
    }
}

void Game::Shutdown()
{
    m_Player.Destroy();

    PAL_DestroyTexture(m_BulletTexture);
    PAL_DestroyTexture(m_MeteorTexture);
    PAL_DestroyRenderer(m_Renderer);
    PAL_DestroyContext(m_Context);
    PAL_DestroyWindow(m_Window);
    PAL_Shutdown();
}

void Game::Run()
{
    while (!PAL_WindowShouldClose(m_Window)) {
        PAL_PullEvents();

        if (PAL_GetKeyState(m_Window, PAL_Keys_Space)) {
            m_Player.FireBullet(m_BulletTexture, m_Bullets);
        }

        PAL_Clear({ .2f, .2f, .2f, 1.0f });
        m_Player.Update();
        m_Meteors.Update();
        m_Bullets.Update();

        PAL_RendererFlush(m_Renderer);
        PAL_SwapBuffers();
    }
}

i32 GetRandomNum()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 1);
    i32 number = dist(gen);
    return number;
}