
#pragma once

#include <vector>

class SpriteGroup;
class Sprite
{
private: 
    SpriteGroup* m_Group = nullptr;
    b8 m_Kill = false;

    friend class SpriteGroup;

public:
    virtual ~Sprite() {}

    virtual void Update() {}
    void Kill() { m_Kill = true; }
};

class SpriteGroup
{
private:
    std::vector<Sprite*> m_Sprites;

public:
    SpriteGroup()
    {
        m_Sprites.clear();
    }

    ~SpriteGroup()
    {
        for (Sprite* sprite : m_Sprites) {
            delete sprite;
        }
        m_Sprites.clear();
    }

    void Destroy()
    {
        for (Sprite* sprite : m_Sprites) {
            delete sprite;
        }
        m_Sprites.clear();
    }

    void Add(Sprite* sprite);
    void Pop(Sprite* sprite);
    void Remove(Sprite* sprite);

    template<typename ... Args>
    void Update(Args&& ...args)
    {
        for (auto it = m_Sprites.begin(); it != m_Sprites.end();) {
            (*it)->Update(std::forward<Args>(args)...);
            Sprite* sprite = (*it);
            if (sprite->m_Kill) {
                it = m_Sprites.erase(it); 
                delete sprite;
            }
            else { it++; }
        }
    }
};

