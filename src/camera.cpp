
#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"

void Camera::setPosition(const glm::vec2& position)
{
    m_Position = position;
    calculateView();
}

void Camera::setSize(const glm::vec2& size)
{
    m_Size = size;
    m_Projection = glm::ortho(0.0f, m_Size.x, m_Size.y, 0.0f, -1.0f, 1.0f);
}

void Camera::setRotation(float rotation)
{
    m_Rotation = rotation;
    calculateView();
}

void Camera::calculateView()
{
    m_View = glm::mat4(1.0f);
    glm::translate(m_View, { -m_Position, 0.0f });
    glm::rotate(m_View, m_Rotation, { 0.0f, 0.0f, 1.0f });
    m_ViewProjection = m_Projection * m_View;
}