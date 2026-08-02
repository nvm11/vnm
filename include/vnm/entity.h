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

    // Grab a component of proper type
    template <typename T>
    T *GetComponent()
    {
        // Look for component of right type
        for (auto &component : components)
        {
            if (T *result = dynamic_cast<T *>(component.get()))
            {
                return result;
            }
        }

        return nullptr;
    }

    // Setters
    void SetActive(bool active);

    // Methods that do shiz
    void Initialize();
    void Update(float deltaTime);
    void Render();

    // Manage components
    // Args... as a type makes a parameter pack
    // This is a template that accepts whatever arguments
    // are needed for the generic inserted
    template <typename T, typename... Args>
    T *AddComponent(Args &&...args);

    template<typename T>
    bool RemoveComponent() {
        // Remove all components of specified type
        for (auto it = components.begin(); it != components.end(); ++it) {
            if (dynamic_cast<T*>(it->get())) {
                components.erase(it);
                return true;
            }
        }

        // No component found
        return false;
    }
};