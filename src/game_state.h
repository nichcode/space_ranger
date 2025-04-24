
#pragma once 

#include "state.h"
#include "player.h"
#include "lives.h"

class GameState : public State
{
private:
    Player m_Player;
    Lives m_Lives;

    PAL_Texture* m_BulletTexture = nullptr;
    PAL_Texture* m_MeteorTexture = nullptr;

    SpriteGroup m_Bullets;
    SpriteGroup m_Meteors;

    b8 m_Init;
    u32 m_MeteorCount, m_MaxMeteors;

public:
    GameState() : m_Init(false) {}
    ~GameState() override;

    void Update(PersistData* data) override;

private:
    void Init(PersistData* data);
    void RespawnMeteors(PAL_Renderer* renderer);
    void MeteorPlayerCollisions();
    void BulletMeteorCollisions();
};