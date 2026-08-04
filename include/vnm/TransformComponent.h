#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Component.h"

class TransformComponent : public Component
{
private:
    bool dirty;

    glm::vec3 position = glm::vec3(0.0f);
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 scale = glm::vec3(1.0f);

    glm::mat4 world;

    void RebuildTransform();

public:
    // Getters
    const glm::vec3 &GetPosition() const;
    const glm::quat &GetRotation() const;
    const glm::vec3 &GetScale() const;
    glm::mat4 GetTransform();

    // Setters
    void SetPosition(const glm::vec3 &pos);
    void SetRotation(const glm::quat &rot);
    void SetScale(const glm::vec3 &s);
};