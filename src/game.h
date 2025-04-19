
#pragma once

#include "PAL/PAL.h"
#include "player.h"

class Game
{
private:
    PAL_Window* m_Window;
    PAL_Context* m_Context;
    PAL_Renderer* m_Renderer;

    Player m_Player;
    PAL_Texture* m_BulletTexture;
    SpriteGroup m_Bullets;

public:
    void Init();
    void Shutdown();
    void Run();
};