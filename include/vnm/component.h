#pragma once
#include <memory>

class Entity;

class Component {
    protected:
    std::shared_ptr<Entity> owner;

    public:
    // Destructor
    virtual ~Component() = default;
    
    // Getters
    std::shared_ptr<Entity> GetOwner() const {return owner; }

    // Setters
    void SetOwner(std::shared_ptr<Entity> entity) { owner = entity; }

    // Methods that do things!
    virtual void Initialize() {}
    virtual void Update(float  deltaTime) {}
    virtual void Render() {}
};