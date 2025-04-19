
#include "bullet.h"
#include "PAL/PAL.h"

void Bullet::Init(PAL_Renderer* renderer, PAL_Texture* texture, f32 x, f32 y)
{
    m_Renderer = renderer;
    m_Texture = texture;
    if (!m_Texture) {
        PAL_LogError(PAL_GetError());
        return;
    }
    m_Rect = PAL_GetTextureRect(m_Texture);
    m_Rect.x = x;
    m_Rect.y = y;
    m_Speed = 4.0f;
}

void Bullet::Update()
{
    m_Rect.y -= m_Speed;
    if (m_Rect.y + m_Rect.h < 0) {
        Kill();
    }
    
    PAL_RendererDrawTexture(m_Renderer, m_Rect, m_Texture);
}