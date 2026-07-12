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
#include <vector>
#include <algorithm>
#include <map>

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

// Vulkan validation layers
const std::vector<char const *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

class HelloTriangleApplication
{
public:
    // GLFW objs and properties
    GLFWwindow *window;

    // Vulkan Objects
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;

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
        // Get the required layers
        std::vector<char const *> requiredLayers;
        if (enableValidationLayers)
        {
            requiredLayers.assign(validationLayers.begin(), validationLayers.end());
        }

        // Check if the required layers are supported by the Vulkan implementation.
        auto layerProperties = context.enumerateInstanceLayerProperties();
        auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
                                                       [&layerProperties](auto const &requiredLayer)
                                                       {
                                                           return std::ranges::none_of(
                                                               layerProperties,
                                                               [requiredLayer](auto const &layerProperty)
                                                               { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
                                                       });
        if (unsupportedLayerIt != requiredLayers.end())
        {
            throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
        }

        // Get required extensions
        auto requiredExtensions = getRequiredInstanceExtensions();

        // Check if the extensions are supported
        auto extensionProperties = context.enumerateInstanceExtensionProperties();
        auto unsupportedPropertyIt =
            std::ranges::find_if(requiredExtensions,
                                 [&extensionProperties](auto const &requiredExtension)
                                 {
                                     return std::ranges::none_of(
                                         extensionProperties,
                                         [requiredExtension](auto const &extensionProperty)
                                         { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
                                 });
        if (unsupportedPropertyIt != requiredExtensions.end())
        {
            throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
        }
        // Describe how to create the app
        // Tells Vk what global extensions and validation layers to use
        // Global = entire application, not just the associated device
        vk::InstanceCreateInfo createInfo{
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
            .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
            .ppEnabledExtensionNames = requiredExtensions.data()};

        // Create the instance
        instance = vk::raii::Instance(context, createInfo);
    }

    std::vector<const char *> getRequiredInstanceExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers)
        {
            extensions.push_back(vk::EXTDebugUtilsExtensionName);
        }

        return extensions;
    }

    // Severity allows for comparison operations
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                                          vk::DebugUtilsMessageTypeFlagsEXT type,
                                                          const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                          void *pUserData)
    {
        std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
        return vk::False;
    }

    void SetupDebugMessenger()
    {
        if (!enableValidationLayers)
        {
            return;
        }

        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
        vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{.messageSeverity = severityFlags,   // types of severities callbacks happen for
                                                                              .messageType = messageTypeFlags,    // which types of messages callback is notified of
                                                                              .pfnUserCallback = &DebugCallback}; // pointer to callback function
        debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
    }

    void PickPhysicalDevice()
    {
        auto physicalDevices = vk::raii::PhysicalDevices(instance);
        if (physicalDevices.empty())
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        // Use an ordered map to automatically sort candidates by increasing score
        std::multimap<int, vk::raii::PhysicalDevice> candidates;

        for (const auto &pd : physicalDevices)
        {
            auto deviceProperties = pd.getProperties();
            auto deviceFeatures = pd.getFeatures();
            uint32_t score = 0;

            // Discrete GPUs have a significant performance advantage
            if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            {
                score += 1000;
            }

            // Maximum possible size of textures affects graphics quality
            score += deviceProperties.limits.maxImageDimension2D;

            // Application can't function without geometry shaders
            if (!deviceFeatures.geometryShader)
            {
                continue;
            }
            candidates.insert(std::make_pair(score, pd));
        }

        // Check if the best candidate is suitable at all
        if (!candidates.empty() && candidates.rbegin()->first > 0)
        {
            physicalDevice = candidates.rbegin()->second;
        }
        else
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    bool IsDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice)
    {
        // Get device details
        auto deviceProperties = physicalDevice.getProperties();
        // Get features supported
        auto deviceFeatures = physicalDevice.getFeatures();

        // Check that it is a dedicated gpu
        if (deviceProperties.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
        {
            return false;
        }
        // Uses geometry shaders?
        if (!deviceFeatures.geometryShader)
        {
            return false;
        }

        return true;
    }

    void InitVulkan()
    {
        CreateInstance();
        SetupDebugMessenger();
        PickPhysicalDevice();
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