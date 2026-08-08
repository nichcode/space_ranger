
#pragma once

#include "asset_manager.h"
#include "glm/glm.hpp"

class Player {
public:
    void initialize();
    void destroy();
    void render(Renderer* renderer);

private:
    Texture* m_Texture = nullptr;
    Renderer* m_Renderer = nullptr;
    glm::vec2 m_Position = { 0.0f, 0.0f };
    glm::vec2 m_Size = { 0.0f, 0.0f };
};