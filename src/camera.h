
#pragma once

#include "glm/glm.hpp"

class Camera {
public:
    void setPosition(const glm::vec2& position);
    void setSize(const glm::vec2& size);
    void setRotation(float rotation);

    const glm::vec2& getPosition() const { return m_Position; }
    const glm::vec2& getSize() const { return m_Size; }
    float getRotation() const { return m_Rotation; }

    const glm::mat4& getViewMatrix() const { return m_View; }
    const glm::mat4& getViewProjectionMatrix() const { return m_ViewProjection; }

private:
    void calculateView();

private:
    glm::mat4 m_ViewProjection;
    glm::mat4 m_View;
    glm::mat4 m_Projection;
    glm::vec2 m_Position = { 0.0f, 0.0f};

    glm::vec2 m_Size = { 0.0f, 0.0f };
    float m_Rotation = 0.0f;
};