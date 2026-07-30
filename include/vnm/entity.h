#include <string>
#include <vector>
#include <memory>
#include "component.h"

class Entity
{
private:
    std::string name;
    bool active = true;
    std::vector<std::unique_ptr<Component>> components;
};