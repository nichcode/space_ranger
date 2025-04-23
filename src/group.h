
#pragma once

#include <vector>

class SpriteGroup;
class Sprite
{
private: 
    SpriteGroup* m_Group = nullptr;
    b8 m_Kill = false;

    friend class SpriteGroup;

protected:
    PAL_Rect m_Rect;

public:
    virtual ~Sprite() {}

    virtual void Update() {}
    void Kill() { m_Kill = true; }

    PAL_Rect& GetRect() { return m_Rect; }
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

    std::vector<Sprite*>::iterator begin() { return m_Sprites.begin(); }
    std::vector<Sprite*>::iterator end() { return m_Sprites.end(); }
    std::vector<Sprite*>::reverse_iterator rbegin() { return m_Sprites.rbegin(); }
    std::vector<Sprite*>::reverse_iterator rend() { return m_Sprites.rend(); }

    std::vector<Sprite*>::const_iterator begin() const { return m_Sprites.begin(); }
    std::vector<Sprite*>::const_iterator end()	const { return m_Sprites.end(); }
    std::vector<Sprite*>::const_reverse_iterator rbegin() const { return m_Sprites.rbegin(); }
    std::vector<Sprite*>::const_reverse_iterator rend() const { return m_Sprites.rend(); }
};

