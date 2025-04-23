
#include "lives.h"
#include "PAL/PAL.h"

void Lives::Init(PAL_Renderer* renderer)
{
    m_Renderer = renderer;
    m_Texture = PAL_LoadTexture("assets/life.png");
    if (!m_Texture) {
        PAL_LogError(PAL_GetError());
        return;
    }
    f32 width = PAL_GetTextureWidth(m_Texture);
    f32 height = PAL_GetTextureHeight(m_Texture);

    for (u32 x = 0; x < 3; x++) {
        m_Rects[x].y = 20.0f; 
        m_Rects[x].w = width; 
        m_Rects[x].h = height; 
    }

    m_Rects[0].x = 640.0f - (1.0f * width);
    m_Rects[1].x = 640.0f - (2.0f * width);
    m_Rects[2].x = 640.0f - (3.0f * width);
    m_Count = 3;
}

void Lives::Destroy()
{
    PAL_DestroyTexture(m_Texture);
}

void Lives::Hit()
{
    // TODO: do not cap lives to one
    if (m_Count != 1) {
        m_Count--;
    }

    // blink player and make player inviscible for a few seconds
}

void Lives::Update()
{
    for (u32 x = 0; x < m_Count; x++) {
        PAL_RendererDrawTexture(m_Renderer, m_Rects[x], m_Texture);
    }
}