
#pragma once

#include "PAL/PAL_window.h"
#include "bullet.h"

class Player
{
private:
    PAL_Window* m_Window;
    PAL_Renderer* m_Renderer;
    PAL_Texture* m_Texture;
    PAL_Rect m_Rect;

    f32 m_Speed, m_Velocity;
    f32 m_ShootCooldown;

public:
    Player() : m_Texture(nullptr) {}
    void Init(PAL_Renderer* renderer, PAL_Window* window);
    void Destroy();

    void Update();
    void FireBullet(PAL_Texture* texture, SpriteGroup& bullets);

    PAL_Rect& GetRect() { return m_Rect; }
};