
#include "player.h"
#include "renderer.h"
#include "helper.h"

void Player::initialize()
{
    m_Texture = AssetManager::loadTexture("assets/textures/player.png");
    if (!m_Texture) {
        palLog(nullptr, "Failed to load player texture");
        DEBUG_BREAK();
        return;
    }

    m_Size = { 128.0f, 128.0f };
    m_Position.x = (WORLD_WIDTH / 2.0f) - m_Size.x;
    m_Position.y = WORLD_HEIGHT - (m_Size.y * 2.0f);
}

void Player::destroy()
{
    AssetManager::destroyTexture(m_Texture);
}

void Player::render(Renderer* renderer)
{
    renderer->drawQuad(m_Position, m_Size, m_Texture);
}