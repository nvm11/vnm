#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define NOMINMAX
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cassert>
#include <stdexcept>
#include <cstdlib>
#include <cstdint>
#include <limits>
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
    vk::raii::SurfaceKHR surface = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    vk::raii::Device device = nullptr;
    vk::raii::Queue queue = nullptr;

    // Swap Chain
    vk::raii::SwapchainKHR swapChain = nullptr;
    std::vector<vk::Image> swapChainImages;
    vk::SurfaceFormatKHR swapChainSurfaceFormat;
    vk::Extent2D swapChainExtent;

    std::vector<const char *> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

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
            .ppEnabledLayerNames = requiredLayers.data(),
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
            // Skip if device isn't suitable
            // Application can't function without geometry shaders
            if (!IsDeviceSuitable(pd))
            {
                continue;
            }

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
        // Check if the physicalDevice supports the Vulkan 1.3 API version
        bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;

        // Check if any of the queue families support graphics operations
        auto queueFamilies = physicalDevice.getQueueFamilyProperties();
        bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const &qfp)
                                                    { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

        // Check if all required physicalDevice extensions are available
        auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
        bool supportsAllRequiredExtensions =
            std::ranges::all_of(requiredDeviceExtension,
                                [&availableDeviceExtensions](auto const &requiredDeviceExtension)
                                {
                                    return std::ranges::any_of(availableDeviceExtensions,
                                                               [requiredDeviceExtension](auto const &availableDeviceExtension)
                                                               { return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
                                });

        // Check if the physicalDevice supports the required features (shader draw parameters, dynamic rendering and extended dynamic state)
        auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                                             vk::PhysicalDeviceVulkan11Features,
                                                             vk::PhysicalDeviceVulkan13Features,
                                                             vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                                        features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                        features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

        // Return true if the physicalDevice meets all the criteria
        return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
    }

    void CreateLogicalDevice()
    {
        // find the index of the first queue family that supports graphics
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

        // get the first index into queueFamilyProperties which supports both graphics and present
        uint32_t queueIndex = ~0;
        for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
        {
            if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
                physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
            {
                // found a queue family that supports both graphics and present
                queueIndex = qfpIndex;
                break;
            }
        }
        if (queueIndex == ~0)
        {
            throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
        }

        // query for Vulkan 1.3 features
        vk::StructureChain<vk::PhysicalDeviceFeatures2,
                           vk::PhysicalDeviceVulkan11Features,
                           vk::PhysicalDeviceVulkan13Features,
                           vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
            featureChain = {
                {},                             // vk::PhysicalDeviceFeatures2
                {.shaderDrawParameters = true}, // vk::PhysicalDeviceVulkan11Features
                {.dynamicRendering = true},     // vk::PhysicalDeviceVulkan13Features
                {.extendedDynamicState = true}  // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
            };

        // create a Device
        float queuePriority = 0.5f;
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo{.queueFamilyIndex = queueIndex, .queueCount = 1, .pQueuePriorities = &queuePriority};
        vk::DeviceCreateInfo deviceCreateInfo{.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
                                              .queueCreateInfoCount = 1,
                                              .pQueueCreateInfos = &deviceQueueCreateInfo,
                                              .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
                                              .ppEnabledExtensionNames = requiredDeviceExtension.data()};

        device = vk::raii::Device(physicalDevice, deviceCreateInfo);
        queue = vk::raii::Queue(device, queueIndex, 0);
    }

    vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats)
    {
        // Ensure we have formats
        assert(!availableFormats.empty());

        // Look for SRGB Format
        const auto formatIt = std::ranges::find_if(
            availableFormats,
            [](const auto &format)
            { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });

        // If desired format isn't found, return the first found one
        return availableFormats[0];
    }

    vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes)
    {
        // eFifo is the only guaranteed mode.
        // We need to see if eMailbox is available
        assert(std::ranges::any_of(availablePresentModes, [](auto presentMode)
                                   { return presentMode == vk::PresentModeKHR::eFifo; }));
        return std::ranges::any_of(availablePresentModes,
                                   [](const vk::PresentModeKHR value)
                                   { return vk::PresentModeKHR::eMailbox == value; })
                   ? vk::PresentModeKHR::eMailbox
                   : vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        return {
            std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
    }

    uint32_t ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities)
    {
        auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
        if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
        {
            minImageCount = surfaceCapabilities.maxImageCount;
        }
        return minImageCount;
    }

    void CreateSwapChain()
    {
        vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
        swapChainExtent = ChooseSwapExtent(surfaceCapabilities);
        uint32_t minImageCount = ChooseSwapMinImageCount(surfaceCapabilities);

        std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
        swapChainSurfaceFormat = ChooseSwapSurfaceFormat(availableFormats);

        std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);

        vk::SwapchainCreateInfoKHR swapChainCreateInfo{.surface = *surface,
                                                       .minImageCount = minImageCount,
                                                       .imageFormat = swapChainSurfaceFormat.format,
                                                       .imageColorSpace = swapChainSurfaceFormat.colorSpace,
                                                       .imageExtent = swapChainExtent,
                                                       .imageArrayLayers = 1,                                    // Generating 2D Output
                                                       .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,   // Operations we'l perform (just drawing to it for now)
                                                       .imageSharingMode = vk::SharingMode::eExclusive,          // How to handle sc images accessed by multiple queues
                                                       .preTransform = surfaceCapabilities.currentTransform,     // Apply transformation before present?
                                                       .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque, // Use alpha for blending?
                                                       .presentMode = chooseSwapPresentMode(availablePresentModes),
                                                       .clipped = true,          // Ignore offscreen pixels?
                                                       .oldSwapchain = nullptr}; // TODO: Account for resize

        swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
        swapChainImages = swapChain.getImages();
    }

    void CreateSurface()
    {
        VkSurfaceKHR _surface;
        // Use glfw to try to create a surface
        if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface))
        {
            throw std::runtime_error("Falied to create window surface");
        }

        // Create actual vulkan surface with C++ wrapper
        surface = vk::raii::SurfaceKHR(instance, _surface);
    }

    void InitVulkan()
    {
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapChain();
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