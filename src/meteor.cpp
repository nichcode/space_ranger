
#include "meteor.h"
#include "game.h"
#include "PAL/PAL.h"

static f32 s_PosX[5] = { 100.0f, 200.0, 300.0f, 400.0f, 500.0f };
static f32 s_PosY[5] = { -100.0f, -120.0, -150.0f, -180.0f, -200.0f };
static u32 s_Index = 0;

void Meteor::Init(PAL_Renderer* renderer, PAL_Texture* texture)
{
    m_Renderer = renderer;
    m_Texture = texture;
    if (!m_Texture) {
        PAL_LogError(PAL_GetError());
        return;
    }
    m_Rect = PAL_GetTextureRect(m_Texture);
    m_Rect.w -= 20.0f;
    m_Rect.h -= 20.0f;
    i32 random = GetRandomNum();

    m_Rect.x = s_PosX[s_Index];
    m_Rect.y = s_PosY[s_Index];
    m_Speed = 4.0f;
    m_SpeedX = 1.0f;

    if (random == 0) { m_Direction = -1.0f; }
    else if (random == 1) { m_Direction = 1.0f; }

    s_Index++;
    if (s_Index == 4) { s_Index = 0; }
}

void Meteor::Update()
{
    m_Rect.x += m_SpeedX * m_Direction;
    m_Rect.y += m_Speed;
    if (m_Rect.y > 480.0f + m_Rect.h) { // screen height
        i32 random = GetRandomNum();
        if (random == 0) { m_Direction = -1.0f; }
        else if (random == 1) { m_Direction = 1.0f; }

        s_Index++;
        if (s_Index == 4) { s_Index = 0; }

        m_Rect.x = s_PosX[s_Index];
        m_Rect.y = s_PosY[s_Index];
    }

    else if (m_Rect.x + m_Rect.w < 0.0f) {
        i32 random = GetRandomNum();
        if (random == 0) { m_Direction = -1.0f; }
        else if (random == 1) { m_Direction = 1.0f; }

        s_Index++;
        if (s_Index == 4) { s_Index = 0; }

        m_Rect.x = s_PosX[s_Index];
        m_Rect.y = s_PosY[s_Index];
    }

    else if (m_Rect.x > 640.0f + m_Rect.w) {
        i32 random = GetRandomNum();
        if (random == 0) { m_Direction = -1.0f; }
        else if (random == 1) { m_Direction = 1.0f; }

        s_Index++;
        if (s_Index == 4) { s_Index = 0; }

        m_Rect.x = s_PosX[s_Index];
        m_Rect.y = s_PosY[s_Index];
    }
    PAL_RendererDrawTexture(m_Renderer, m_Rect, m_Texture);
}