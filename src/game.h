
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
    PAL_Texture* m_MeteorTexture;
    SpriteGroup m_Bullets;
    SpriteGroup m_Meteors;

    u32 m_MeteorCount, m_MaxMeteors;

public:
    void Init();
    void Shutdown();
    void Run();

    void RespawnMeteors();
    void MeteorPlayerCollisions();
};

i32 GetRandomNum();