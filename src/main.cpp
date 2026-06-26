#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <memory>

class HelloTriangleApplication {
public:

    GLFWwindow* window;
    static constexpr uint32_t WIDTH = 800;
    static constexpr uint32_t HEIGHT = 600;


    void run() {
        InitWindow();
        InitVulkan();
        mainLoop();
        cleanup();
    }

private:
    void InitWindow() {
        // Init GLFW library
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // Disable resized windows for now
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        // Create actual window
        window = glfwCreateWindow(
            WIDTH,
            HEIGHT,
            "Vulkan",   // Title
            nullptr,    // Primary monitor for fullscreen
            nullptr);   // Handle to another shared window
    }

    void InitVulkan() {

    }

    void mainLoop() {
        // Keep going?
        while(!glfwWindowShouldClose(window)){
            glfwPollEvents();
        }
    }

    void cleanup() {
        // Destroy window and GLFW
        glfwDestroyWindow(window);
        glfwTerminate();

    }
};

int main()
{
    try
    {
        HelloTriangleApplication app;
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}