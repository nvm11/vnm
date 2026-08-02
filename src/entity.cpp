#include "Entity.h"
#include "Component.h"

// Constructor
Entity::Entity(const std::string &name) : name(name) {}

// Getters
bool Entity::IsActive()
{
    return active;
}

const std::string &Entity::GetName()
{
    return name;
}

// Setters
void Entity::SetActive(bool active)
{
    this->active = active;
}

// Methods that do stuff
// Setup
void Entity::Initialize()
{
    for (auto &component : components)
    {
        component->Initialize();
    }
}

// Game Loop
void Entity::Update(float deltaTime)
{
    if (!active)
        return;

    // Update all components
    for (auto &component : components)
    {
        component->Update(deltaTime);
    }
}

void Entity::Render()
{
    if (!active)
        return;

    // Render all components
    for (auto &component : components)
    {
        component->Render();
    }
}

template <typename T, typename... Args>
T *Entity::AddComponent(Args &&...args)
{
    // Check if it is a component
    static_assert(std::is_base_of<Component, T>::value, "Added type must derive from Component");

    // Create a component of the given type
    // With passed Args
    // Casts each arg into the original value category
    // ... expands the pack passed in
    auto component = std::make_unique<T>(std::forward<Args>(args)...);
    auto pointer = component.get();
    pointer->SetOwner(this);
    components.push_back(std::move(component));

    // Return a reference that can be modified
    return pointer;
}