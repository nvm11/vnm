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

class HelloTriangleApplication
{
public:
    // GLFW objs and properties
    static constexpr uint32_t WIDTH = 800;
    static constexpr uint32_t HEIGHT = 600;
    GLFWwindow *window;

    // Vulkan Objects
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;

    void Run()
    {
        InitWindow();
        InitVulkan();
        MainLoop();
        Cleanup();
    }

private:
    void InitWindow()
    {
        // Init GLFW library
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // Disable resized windows for now
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        // Create actual window
        window = glfwCreateWindow(
            WIDTH,
            HEIGHT,
            "Vulkan", // Title
            nullptr,  // Primary monitor for fullscreen
            nullptr); // Handle to another shared window
    }

    void CreateInstance()
    {

        constexpr vk::ApplicationInfo appInfo{
            .pApplicationName = "Hello Triangle",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = vk::ApiVersion14};

        // Get the required instance extensions
        // Extension interface helps with the platform agnositc aspect of Vulkan
        uint32_t glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        // Check if required GLFW extensions are supported by VK implementation
        auto extensionProps = context.enumerateInstanceExtensionProperties();
        for (uint32_t i = 0; i < glfwExtensionCount; ++i)
        {
            if (std::ranges::none_of(extensionProps,
                                     [glfwExtension = glfwExtensions[i]](auto const &extensionProperty)
                                     { return strcmp(extensionProperty.extensionName, glfwExtension) == 0; }))
            {
                throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
            }
        }

        // Describe how to create the app
        // Tells Vk what global extensions and validation layers to use
        // Global = entire application, not just the associated device
        vk::InstanceCreateInfo createInfo{
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = glfwExtensionCount,
            .ppEnabledExtensionNames = glfwExtensions};

        
        // Create the instance
        instance = vk::raii::Instance(context, createInfo);
    }

    void InitVulkan()
    {
        CreateInstance();
    }

    void MainLoop()
    {
        // Keep going?
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
        }
    }

    void Cleanup()
    {
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
        app.Run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}