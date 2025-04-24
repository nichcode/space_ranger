
#include "game_state.h"
#include "PAL/PAL.h"
#include "meteor.h"

#include <random>

i32 GetRandomNum()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 1);
    i32 number = dist(gen);
    return number;
}

void GameState::Init(PersistData* data)
{
    m_Init = false;
    m_BulletTexture = PAL_LoadTexture("assets/bullet.png");
    m_MeteorTexture = PAL_LoadTexture("assets/meteor.png");

    m_Player.Init(data->renderer, data->window);

    // create meteors
    m_MaxMeteors = 40;
    m_MeteorCount = 0;
    while (m_MeteorCount < m_MaxMeteors) {
        Meteor* meteor = new Meteor();
        meteor->Init(data->renderer, m_MeteorTexture);
        m_Meteors.Add(meteor);
        m_MeteorCount++;
    }

    m_Lives.Init(data->renderer);
}

GameState::~GameState()
{
    m_Player.Destroy();
    m_Lives.Destroy();
    
    if (m_BulletTexture) {
        PAL_DestroyTexture(m_BulletTexture);
    }
    if (m_MeteorTexture) {
        PAL_DestroyTexture(m_MeteorTexture);
    }
}

void GameState::Update(PersistData* data)
{
    if (!m_Init) {
        Init(data);
        m_Init = true;
    }
    PAL_Clear({.2f, .2f, .2f, 1.0f});

    m_Player.FireBullet(m_BulletTexture, m_Bullets);
    RespawnMeteors(data->renderer);
    MeteorPlayerCollisions();
    BulletMeteorCollisions();

    PAL_Clear({ .2f, .2f, .2f, 1.0f });
    m_Player.Update();
    m_Meteors.Update();
    m_Bullets.Update();
    m_Lives.Update();
}

void GameState::RespawnMeteors(PAL_Renderer* renderer)
{
    while (m_MeteorCount < m_MaxMeteors) {
        Meteor* meteor = new Meteor();
        meteor->Init(renderer, m_MeteorTexture);
        m_Meteors.Add(meteor);
        m_MeteorCount++;
    }
}

void GameState::MeteorPlayerCollisions()
{
    for (Sprite* meteor : m_Meteors) {
        PAL_Rect& rect = meteor->GetRect();
        PAL_Rect& player_rect = m_Player.GetRect();
        if (PAL_RectCollide(player_rect, rect)) {
            meteor->Kill();
            m_Lives.Hit();
            m_MeteorCount--;
        }
    }
}

void GameState::BulletMeteorCollisions()
{
    for (Sprite* bullet : m_Bullets) {
        for (Sprite* meteor : m_Meteors) {
            PAL_Rect& bullet_rect = bullet->GetRect();
            PAL_Rect& meteor_rect = meteor->GetRect();
            if (PAL_RectCollide(bullet_rect, meteor_rect)) {
                meteor->Kill();
                bullet->Kill();
                m_MeteorCount--;
            }
        }
    }
}
