
#pragma once

#include "PAL/PAL_renderer.h"
#include "group.h"

class Meteor : public Sprite
{
private:
    PAL_Renderer* m_Renderer;
    PAL_Texture* m_Texture;
    PAL_Rect m_Rect;

    f32 m_Speed, m_Direction, m_SpeedX;

public:
    void Init(PAL_Renderer* renderer, PAL_Texture* texture);
    void Update();
};