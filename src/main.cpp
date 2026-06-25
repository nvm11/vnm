#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return 1;
    }

    if (!glfwVulkanSupported()) {
        std::cerr << "Vulkan not supported\n";
        return 1;
    }

    std::cout << "Vulkan is supported. Ready to go.\n";

    glfwTerminate();
    return 0;
}