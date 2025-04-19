
#include "player.h"
#include "PAL/PAL.h"

void Player::Init(PAL_Renderer* renderer, PAL_Window* window) 
{
    m_Renderer = renderer;
    m_Window = window;
    m_Texture = PAL_LoadTexture("assets/player.png");
    if (!m_Texture) {
        PAL_LogError(PAL_GetError());
        return;
    }
    m_Rect = PAL_GetTextureRect(m_Texture);

    m_Rect.x = 320.0f - m_Rect.w / 2.0f;
    m_Rect.y = 350.0f;
    m_Speed = 4.0f;

    m_ShootCooldown = 0.0f;
}

void Player::Destroy() 
{
    PAL_DestroyTexture(m_Texture);
}

void Player::Update()
{
    m_Velocity = 0.0f;
    if (PAL_GetKeyState(m_Window, PAL_Keys_Right)) {
        m_Velocity = m_Speed;
    }

    else if (PAL_GetKeyState(m_Window, PAL_Keys_Left)) {
        m_Velocity = -m_Speed;
    }

    if (m_ShootCooldown > 0.0f) {
        m_ShootCooldown--;
    }

    if (m_Rect.x - m_Rect.w > 640.0f) {
        m_Rect.x = - m_Rect.w;
    }

    if (m_Rect.x + m_Rect.w < 0.0f) {
        m_Rect.x = 640.0f + m_Rect.w;
    }

    m_Rect.x += m_Velocity;
    PAL_RendererDrawTexture(m_Renderer, m_Rect, m_Texture);
}

void Player::FireBullet(PAL_Texture* texture, SpriteGroup& bullets)
{
    if (m_ShootCooldown <= 0.0f) {
        Bullet* bullet = new Bullet();
        f32 x = m_Rect.x + (m_Rect.w / 2.0f) - 7.0f;
        f32 y = m_Rect.y + (m_Rect.h / 2.0f) - 90.0f;
        bullet->Init(m_Renderer, texture, x, y);
        bullets.Add(bullet);
        m_ShootCooldown = 20.0f;
    }
}