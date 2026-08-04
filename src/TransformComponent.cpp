#include "TransformComponent.h"

// Getters
const glm::vec3 &TransformComponent::GetPosition() const { return position; }
const glm::quat &TransformComponent::GetRotation() const { return rotation; }
const glm::vec3 &TransformComponent::GetScale() const { return scale; }

// Setters
void TransformComponent::SetPosition(const glm::vec3 &pos)
{
    position = pos;
    dirty = true;
}
void TransformComponent::SetRotation(const glm::quat &rot)
{
    rotation = rot;
    dirty = true;
}
void TransformComponent::SetScale(const glm::vec3 &s)
{
    scale = s;
    dirty = true;
}

// World Matrix
void TransformComponent::RebuildTransform()
{
    auto new_position = glm::translate(glm::mat4(1.0f), position);
    auto new_rotation = glm::mat4_cast(rotation);
    auto new_scale = glm::scale(glm::mat4(1.0f), scale);

    world = new_position * new_rotation * new_scale;
    dirty = false;
}

glm::mat4 TransformComponent::GetTransform()
{
    if (dirty)
    {
        RebuildTransform();
    }

    return world;
}