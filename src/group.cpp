
#include "group.h"
#include "PAL/PAL_log.h"

#include <algorithm>

void SpriteGroup::Add(Sprite* sprite)
{
    if (!sprite) { PAL_LogError("sprite is null"); }
    sprite->m_Group = this;
    m_Sprites.push_back(sprite);
}

void SpriteGroup::Pop(Sprite* sprite)
{
    auto it = std::find(m_Sprites.begin(), m_Sprites.end(), sprite);
    if (it != m_Sprites.end()) {
        m_Sprites.erase(it); 
    }

    delete sprite;
    sprite = nullptr;
}

void SpriteGroup::Remove(Sprite* sprite)
{
    auto sprite_it = std::find(m_Sprites.begin(), m_Sprites.end(), sprite);
    for (auto it = m_Sprites.begin(); it != m_Sprites.end();) {
        if (it == sprite_it) {
            it = m_Sprites.erase(it); 
            delete (*it);
            (*it) = nullptr;
        }
        else { it++; }
    }
}