
#pragma once

#include "PAL/PAL_renderer.h"
#include "group.h"

class Bullet : public Sprite
{
private:
    PAL_Renderer* m_Renderer;
    PAL_Texture* m_Texture;

    f32 m_Speed;

public:
    void Init(PAL_Renderer* renderer, PAL_Texture* texture, f32 x, f32 y);
    void Update();
};