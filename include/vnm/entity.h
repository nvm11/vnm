#pragma once
#include <string>
#include <vector>
#include <memory>

#include "Component.h"

class Entity
{
private:
    // Fields
    std::string name;
    bool active = true;
    std::vector<std::unique_ptr<Component>> components;

public:
    // Explicit keyword requires exact type matching
    // Constructor
    explicit Entity(const std::string &name);

    // Getters
    const std::string &GetName();
    bool IsActive();

    // Setters
    void SetActive(bool active);
};