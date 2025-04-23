
#pragma once

#include "PAL/PAL_renderer.h"

class Lives
{
private:
    PAL_Renderer* m_Renderer;
    PAL_Texture* m_Texture;
    PAL_Rect m_Rects[3];

    u32 m_Count;

public:
    void Init(PAL_Renderer* renderer);
    void Destroy();

    void Hit();
    void Update();
};