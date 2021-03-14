#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <memory>

#include <array>
#include <vector>
#include <set>

#include <optional>
#include <chrono>

#include <Vfs.h>



#include <CommonUtl.h>
#include <TFVulkanDevice.h>
#include <VulkanCoreHelper.h>
#include <VulkanPipelineHelper.h>
#include <VulkanBufferHelper.h>
#include <TFVulkanDevice.h>
#include "TextureManager.h"

#include <GltfLoader.h>
#include <SkySystem.h>
#include <TerrainSystem.h>
#include <ObjModel.h>
#include <World.h>
#include <HardCodeScene.h>





const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;
const int MAX_FRAMES_IN_FLIGHT = 2;
static float t = 0;

// #pragma region 

const std::vector<Vertex> vertices = {
    {{-0.5f, 0, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
};

// 16_t for less than 65535 vertices
const std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0,
};

struct CameraUniformBufferObject
{
    float4x4 view;
    float4x4 proj;
    float4x4 invVPF; // inverse view projection framebuffer;
    float4 cameraPos;
    float4 lightDir;
    float4 lightColor;
};

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

struct QueueFamilyIndices 
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanRendererApp
{
public:
    

    void run()
    {
        initWindow();
        initVulkan();
        initImGui();
        //m_terrainSystem.Init(m_tfDevice, m_descriptorPool,m_renderPass, m_swapChainExtent, m_pipelineLayout, m_descriptorSetLayoutTextures);
        m_skySystem.Init(m_tfDevice, m_renderPass, m_swapChainExtent, m_pipelineLayout);
        m_world.Init();
        TF::SpawnObjects(&m_world);
        mainLoop();
        cleanup();
    }
private:
#pragma region member
    GLFWwindow* m_window = nullptr;

    SkySystem m_skySystem;
    TerrainSystem m_terrainSystem;

    TF::World m_world;

    TF::TFVulkanDevice m_tfDevice;
    VkInstance m_instance;
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger;

    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImageView> m_swapChainImageViews;

    VkImage m_depthImage;
    VkDeviceMemory m_depthImageMemory;
    VkImageView m_depthImageView;

    VkBuffer m_vertexBuffer;
    VkDeviceMemory  m_vertexBufferMemory;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;

    GltfModel* m_gltfModel;

    VkBuffer m_stagingBuffer;
    VkDeviceMemory m_stagingBufferMemory;
    uint32_t m_mipLevels;
    VkImage m_textureImage;
    VkDeviceMemory m_textureImageMemory;
    VkImageView m_textureImageView;
    VkSampler m_textureSampler;

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;

    

    VkDescriptorSetLayout m_descriptorSetLayoutMatrices;
    VkDescriptorSetLayout m_descriptorSetLayoutTextures;
    VkPipelineLayout m_pipelineLayout;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorPool m_descriptorPoolImGui;
    std::vector<VkDescriptorSet> m_descriptorSets;


    VkRenderPass m_renderPass;
    VkRenderPass m_renderPassImGui;
    VkPipeline m_graphicsPipeline;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    std::vector<VkFramebuffer> m_imGuiFrameBuffers;
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VkCommandBuffer> m_imGuiCommandBuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    size_t m_currentFrame = 0;

    ImGui_ImplVulkanH_Window m_mainWindowData;

    float m_cameraAngle = 0;//89;
    float m_cameraDistance = 5;//800;
    float m_cameraOffsetX = 0;
    float3 m_lightDir{0,1,0};
    float3 m_lightColor{1,1,1};
    float m_lightIntensity = 1.0f;


    bool m_framebufferResized = false;
#pragma endregion

    VkCommandBuffer beginSingleTimeCommands() {
        return BeginSingleTimeCommands(m_device ,m_commandPool);
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        EndSingleTimeCommands(m_device, m_graphicsQueue, m_commandPool,commandBuffer);
    }
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) 
    {
        auto app = reinterpret_cast<VulkanRendererApp*>(glfwGetWindowUserPointer(window));
        app->m_framebufferResized = true;
    }
    void initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);

    }
#pragma region layerAndExtesion
    bool checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) 
        {
            // iterator through required layer
            // the loop in the vulkan tutorial is a little bit wiered
            
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return extensions;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) 
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }
        return requiredExtensions.empty();
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
    {
        uint32_t formatCount;
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);
        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) 
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT || VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
        createInfo.messageSeverity =  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }
    #pragma endregion
#pragma region queueFamily
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;
        // Logic to find queue family indices to populate struct with

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
            if (presentSupport)
            {
                indices.presentFamily = i;
            }
            i++;
        }
        

        return indices;
    }
#pragma endregion
#pragma region surfaceAndPresent
    void createSurface()
    {
        if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface");
        }
        m_mainWindowData.Surface = m_surface;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat :availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(m_window, &width, &height);

            VkExtent2D actualExtent = 
            {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };
            actualExtent.width = std::max(capabilities.minImageExtent.width,std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height,std::min(capabilities.maxImageExtent.height, actualExtent.height));
            return actualExtent;
        }
        
    }
    
    void createSwapChain() 
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        m_mainWindowData.SurfaceFormat = surfaceFormat;
        m_mainWindowData.PresentMode = presentMode;

        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) 
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } 
        else 
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
        m_swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());
        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;

    }
    void createImageViews() 
    {
        m_swapChainImageViews.resize(m_swapChainImages.size());
        for (size_t i = 0; i < m_swapChainImages.size(); i++)
        {
           m_swapChainImageViews[i] = CreateImageView(m_device,m_swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT,1);
        }
    }

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
    {
        for (VkFormat format : candidates) 
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
            {
                return format;
            } 
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
            {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat findDepthFormat() {
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
    }

    bool hasStencilComponent(VkFormat format) 
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }


    // https://vulkan-tutorial.com/Depth_buffering
    // when need explicit depth transitiong
    void createDepthResources() 
    {
        VkFormat depthFormat = findDepthFormat();
        CreateImage(m_physicalDevice, m_device, m_swapChainExtent.width, m_swapChainExtent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);
        m_depthImageView = CreateImageView(m_device, m_depthImage, depthFormat,VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    }

    
#pragma endregion
#pragma region device
    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }
        
        // most case there is only one discrete gpu, so this should be fine for now.
        return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                && indices.isComplete()
                && extensionsSupported 
                && swapChainAdequate
                && deviceFeatures.samplerAnisotropy;
    }
    void pickPhysicalDevice() 
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
        if (deviceCount == 0) 
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
        for (const auto& device : devices)
        {
            if (isDeviceSuitable(device))
            {
                m_physicalDevice = device;
                break;
            }
        }
        if (m_physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

    }

    void createLogicalDevice() 
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value()};
        

        float queuePriority = 1.0f;

        for (uint32_t queueFamily: uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();



        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }
        
        if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(m_device, queueFamilyIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, queueFamilyIndices.presentFamily.value(),  0, &m_presentQueue);



    }
#pragma endregion
#pragma region instance

    void createInstance()
    {

        if (enableValidationLayers && !checkValidationLayerSupport()) 
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "VulkanRenderer";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;
        
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;     

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
         
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
            // enable best practice
            VkValidationFeatureEnableEXT enables[] = {VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT};
            VkValidationFeaturesEXT features = {};
            features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
            features.enabledValidationFeatureCount = 1;
            features.pEnabledValidationFeatures = enables;
            debugCreateInfo.pNext = &features;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            
            createInfo.pNext = nullptr;
        }             
        
        if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }

        uint32_t totalExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &totalExtensionCount, nullptr);
        std::vector<VkExtensionProperties> totalExtensions(totalExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &totalExtensionCount, totalExtensions.data());
        std::cout << "available extensions:" << std::endl;


        for (const auto& extension : totalExtensions) {
            std::cout << '\t' << extension.extensionName << std::endl;
        }

    }
#pragma endregion
#pragma region buffer
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }
    
    void createVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(m_physicalDevice,m_device,bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        
        void* data;
        vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), bufferSize);
        vkUnmapMemory(m_device, stagingBufferMemory);
        CreateBuffer(m_physicalDevice,m_device,bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertexBuffer, m_vertexBufferMemory);
        copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        vkFreeMemory(m_device, stagingBufferMemory, nullptr);
    }
    void createIndexBuffer() {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(m_physicalDevice,m_device,bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t) bufferSize);
        vkUnmapMemory(m_device, stagingBufferMemory);

        CreateBuffer(m_physicalDevice,m_device,bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

        copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        vkFreeMemory(m_device, stagingBufferMemory, nullptr);
    }

    void createDescriptorSetLayout() 
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

        std::array<VkDescriptorSetLayoutBinding,3> samplerLayoutBinding{};
        for (int i = 0;i<3;i++)
        {
            samplerLayoutBinding[i].binding = i;
            samplerLayoutBinding[i].descriptorCount = 1;
            samplerLayoutBinding[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding[i].pImmutableSamplers = nullptr;
            samplerLayoutBinding[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        



        VkDescriptorSetLayoutCreateInfo uboLayoutInfo{};
        uboLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        uboLayoutInfo.bindingCount = 1;
        uboLayoutInfo.pBindings = &uboLayoutBinding;

        VkDescriptorSetLayoutCreateInfo samplerLayoutInfo{};
        samplerLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        samplerLayoutInfo.bindingCount = 3;
        samplerLayoutInfo.pBindings = samplerLayoutBinding.data();

        if (vkCreateDescriptorSetLayout(m_device, &uboLayoutInfo, nullptr, &m_descriptorSetLayoutMatrices) != VK_SUCCESS) {
            throw std::runtime_error("failed to create matrices descriptor set layout!");
        }

        if (vkCreateDescriptorSetLayout(m_device, &samplerLayoutInfo, nullptr, &m_descriptorSetLayoutTextures) != VK_SUCCESS) {
            throw std::runtime_error("failed to create textures descriptor set layout!");
        }
    }

    void createUniformBuffers() 
    {
        VkDeviceSize bufferSize = sizeof(CameraUniformBufferObject);

        m_uniformBuffers.resize(m_swapChainImages.size());
        m_uniformBuffersMemory.resize(m_swapChainImages.size());

        for (size_t i = 0; i < m_swapChainImages.size(); i++) 
        {
            CreateBuffer(m_physicalDevice,m_device,bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
        }
    }

    void updateUniformBuffer(uint32_t currentImage) 
    {
        // update camera buffer
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        t = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        CameraUniformBufferObject ubo{};
        
        //ubo.model = Matrix::CreateRotationY(time);//.Transpose();
        float cameraAngleRad = m_cameraAngle / 180.0 * 3.1415926;
        float4 cameraPos = float4(m_cameraOffsetX,sin(cameraAngleRad)*m_cameraDistance,-cos(cameraAngleRad)*m_cameraDistance,0);
        
        ubo.view = XMMatrixLookAtLH(cameraPos, float3(0,0,0), float3(0,1,0));
        //ubo.view = ubo.view;//.Transpose();
        ubo.proj = DirectX::XMMatrixPerspectiveFovLH(3.14f / 2.0f, m_swapChainExtent.width / (float) m_swapChainExtent.height, 1.0f, 10000.0f);
        //ubo.proj = ubo.proj.Transpose();
        float3 unitLightDir = m_lightDir;
        unitLightDir.Normalize();
        ubo.lightDir = float4(unitLightDir.x,unitLightDir.y,unitLightDir.z,1);
        ubo.lightColor = float4(m_lightColor.x*m_lightIntensity, m_lightColor.y*m_lightIntensity, m_lightColor.z*m_lightIntensity, m_lightIntensity);

        float w = m_swapChainExtent.width;
        float h = m_swapChainExtent.height;
        float4x4 clipToFrameBuffer =
        float4x4(
            float4(w,0,0,w*0.5f),
            float4(0,h,0,h*0.5f),
            float4(0,0,0,0),
            float4(0,0,0,0)
        );
        
        ubo.invVPF = (ubo.view * ubo.proj * clipToFrameBuffer).Invert();
        ubo.cameraPos = cameraPos;
        void* data;
        vkMapMemory(m_device, m_uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(m_device, m_uniformBuffersMemory[currentImage]);
    }

    void generateCommandBuffer(uint32_t currentImage)
    {
        // clear command buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(m_commandBuffers[currentImage], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_swapChainFramebuffers[currentImage];

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_commandBuffers[currentImage], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(m_commandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
        VkBuffer vertexBuffers[] = {m_vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(m_commandBuffers[currentImage], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(m_commandBuffers[currentImage], m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(m_commandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[currentImage], 0, nullptr);
        // for (int j = 0;j <4;j++)
        // {
        //     float4x4 modelMatrix = Matrix::CreateTranslation(float3(0,j*0.5f,0));
        //     vkCmdPushConstants(
        //                     m_commandBuffers[i],
        //                     m_pipelineLayout,
        //                     VK_SHADER_STAGE_VERTEX_BIT,
        //                     0,
        //                     sizeof(float4x4),
        //                     &modelMatrix);
        //     vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        // }
        float4x4 modelMatrix = Matrix::CreateScale(m_gltfModel->m_scale);
        modelMatrix *= Matrix::CreateRotationY(t);
        modelMatrix *= Matrix::CreateTranslation(float3(0,-2.0f,0));
        vkCmdPushConstants(
                            m_commandBuffers[currentImage],
                            m_pipelineLayout,
                            VK_SHADER_STAGE_VERTEX_BIT,
                            0,
                            sizeof(float4x4),
                            &modelMatrix);
        //vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        m_gltfModel->Draw(m_commandBuffers[currentImage],m_pipelineLayout);
        //m_terrainSystem.Draw(m_commandBuffers[currentImage],m_pipelineLayout);
        m_skySystem.Draw(m_commandBuffers[currentImage],m_pipelineLayout);
        
        
        vkCmdEndRenderPass(m_commandBuffers[currentImage]);
        if (vkEndCommandBuffer(m_commandBuffers[currentImage]) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void generateImGuiCommandBuffer(uint32_t currentImage, ImDrawData* draw_data)
    {
        // clear command buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(m_imGuiCommandBuffers[currentImage], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPassImGui;
        renderPassInfo.framebuffer = m_imGuiFrameBuffers[currentImage];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapChainExtent;
        vkCmdBeginRenderPass(m_imGuiCommandBuffers[currentImage], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_RenderDrawData(draw_data, m_imGuiCommandBuffers[currentImage]);

        vkCmdEndRenderPass(m_imGuiCommandBuffers[currentImage]);
        if (vkEndCommandBuffer(m_imGuiCommandBuffers[currentImage]) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to record imgui command buffer!");
        }

    }

    void createDescriptorPool() 
    {
        // descPool for renderer
        {
            std::array<VkDescriptorPoolSize, 2> poolSizes{};
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapChainImages.size());
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[1].descriptorCount = static_cast<uint32_t>(m_gltfModel->m_images.size());

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = static_cast<uint32_t>(1024);
            poolInfo.flags = 0;

            if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to create descriptor pool!");
            }
        }
        // descPool for imgui
        {
            VkDescriptorPoolSize pool_sizes[] =
            {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
            pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;
            auto err = vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_descriptorPoolImGui);
            check_vk_result(err);
        }
        
    }

    void createDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(m_swapChainImages.size(), m_descriptorSetLayoutMatrices);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(m_swapChainImages.size());
        allocInfo.pSetLayouts = layouts.data();

        m_descriptorSets.resize(m_swapChainImages.size());
        if (vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }
        for (size_t i = 0; i < m_swapChainImages.size(); i++) 
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(CameraUniformBufferObject);

            // VkDescriptorImageInfo imageInfo{};
            // imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            // imageInfo.imageView = m_textureImageView;
            // imageInfo.sampler = m_textureSampler;

            std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = m_descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            // descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            // descriptorWrites[1].dstSet = m_descriptorSets[i];
            // descriptorWrites[1].dstBinding = 1;
            // descriptorWrites[1].dstArrayElement = 0;
            // descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            // descriptorWrites[1].descriptorCount = 1;
            // descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }

        for (auto& mat : m_gltfModel->m_materials)
        {
            VkDescriptorSetAllocateInfo samplerAllocInfo{};
            samplerAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            samplerAllocInfo.descriptorPool = m_descriptorPool;
            samplerAllocInfo.descriptorSetCount = 1;
            samplerAllocInfo.pSetLayouts = &m_descriptorSetLayoutTextures;
            vkAllocateDescriptorSets(m_device, &samplerAllocInfo, &mat.descriptorSet);
            std::vector<VkWriteDescriptorSet> writeDescriptorSets = 
            {
                WriteDescriptorSet(mat.descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &m_gltfModel->m_images[mat.baseColorTextureIndex].texture.m_descriptor),
			    WriteDescriptorSet(mat.descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &m_gltfModel->m_images[mat.normalTextureIndex].texture.m_descriptor),
                WriteDescriptorSet(mat.descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &m_gltfModel->m_images[mat.metallicRoughnessTextureIndex].texture.m_descriptor),
            };
			vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
        }
    }
#pragma endregion
#pragma region texture
    void createTextureImage ()
    {
        std::string imgPath("../../res/model/FlightHelmet/FlightHelmet_Materials_LensesMat_OcclusionRoughMetal.png");
        auto texSharedPtr = TF::TextureManager::Get()->LoadTexture(imgPath);
        VkDeviceSize imageSize = texSharedPtr->Width() * texSharedPtr->Height() * 4;
        m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texSharedPtr->Width(), texSharedPtr->Height())))) + 1;
        CreateBuffer(m_physicalDevice,m_device,imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_stagingBuffer, m_stagingBufferMemory);
        void* data;
        vkMapMemory(m_device, m_stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, texSharedPtr->Pixels.data(), static_cast<size_t>(imageSize));
        vkUnmapMemory(m_device, m_stagingBufferMemory);

        CreateImage(m_physicalDevice, m_device,texSharedPtr->Width(), texSharedPtr->Height(), m_mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);
        
        TransitionImageLayout(m_device,m_graphicsQueue,m_commandPool,m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,m_mipLevels);
        CopyBufferToImage(m_device,m_graphicsQueue,m_commandPool,m_stagingBuffer, m_textureImage, static_cast<uint32_t>(texSharedPtr->Width()), static_cast<uint32_t>(texSharedPtr->Height()));
        
        //transitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,m_mipLevels);
        GenerateMipmaps(m_physicalDevice,m_device,m_graphicsQueue,m_commandPool,m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, texSharedPtr->Width(), texSharedPtr->Height(), m_mipLevels);
        vkDestroyBuffer(m_device, m_stagingBuffer, nullptr);
        vkFreeMemory(m_device, m_stagingBufferMemory, nullptr);
    }


    void createTextureImageView() 
    {
        //m_textureImageView = CreateImageView(m_device, m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels);
    }

    
#pragma endregion
#pragma region graphicsPipeline
    void createPipelineLayout()
    {
        // Define the push constant range used by the pipeline layout
		// Note that the spec only requires a minimum of 128 bytes, so for passing larger blocks of data you'd use UBOs or SSBOs
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(float4x4);

        std::array<VkDescriptorSetLayout, 2> setLayouts = { m_descriptorSetLayoutMatrices, m_descriptorSetLayoutTextures };
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 2; // Optional
        pipelineLayoutInfo.pSetLayouts = setLayouts.data(); // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 1; // Optional
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; // Optional

        if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void createGraphicsPipeline() 
    {
        auto shaderPath = Vfs::GetShaderPath();
        auto vertShaderCode = ReadFile(Vfs::ConcatPath(shaderPath, "triangle.vert.spv").generic_string());
        auto fragShaderCode = ReadFile(Vfs::ConcatPath(shaderPath, "triangle.frag.spv").generic_string());
        
        PipelineParameter pipelineParameter = PipelineParameter(m_swapChainExtent);

        m_graphicsPipeline = CreateGraphicsPipeline(m_device, m_pipelineLayout, m_renderPass, pipelineParameter, vertShaderCode, fragShaderCode);  
    }
#pragma endregion
#pragma region renderpass
    void createRenderPass() 
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        //colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        subpass.colorAttachmentCount = 1; // I guess this is for mrt
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;


        

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask =  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create render pass!");
        }

        VkAttachmentDescription imGuiColorAttachment{};
        imGuiColorAttachment.format = m_swapChainImageFormat;
        imGuiColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        imGuiColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        imGuiColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        imGuiColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        imGuiColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        imGuiColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        //colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imGuiColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference imGuiColorAttachmentRef{};
        imGuiColorAttachmentRef.attachment = 0;
        imGuiColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDependency imGuiDependency = {};
        imGuiDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        imGuiDependency.dstSubpass = 0;
        imGuiDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        imGuiDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        imGuiDependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        imGuiDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkSubpassDescription imGuiSubpass{};
        imGuiSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        imGuiSubpass.colorAttachmentCount = 1; // I guess this is for mrt
        imGuiSubpass.pColorAttachments = &imGuiColorAttachmentRef;

        VkRenderPassCreateInfo imGuiPassInfo = {};
        imGuiPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        imGuiPassInfo.attachmentCount = 1;
        imGuiPassInfo.pAttachments = &imGuiColorAttachment;
        imGuiPassInfo.subpassCount = 1;
        imGuiPassInfo.pSubpasses = &imGuiSubpass;
        imGuiPassInfo.dependencyCount = 1;
        imGuiPassInfo.pDependencies = &imGuiDependency;
        if (vkCreateRenderPass(m_device, &imGuiPassInfo, nullptr, &m_renderPassImGui) != VK_SUCCESS) {
            throw std::runtime_error("Could not create Dear ImGui's render pass");
        }
    }
#pragma region 

#pragma region framebuffer
    void createFramebuffers() 
    {
        {
            m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
            for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
            {
                std::array<VkImageView, 2> attachments = {
                   m_swapChainImageViews[i],
                    m_depthImageView
                };

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = m_renderPass;
                framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                framebufferInfo.pAttachments = attachments.data();
                framebufferInfo.width = m_swapChainExtent.width;
                framebufferInfo.height = m_swapChainExtent.height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) 
                {
                    throw std::runtime_error("failed to create framebuffer!");
                }
            }
        }

        {
            m_imGuiFrameBuffers.resize(m_swapChainImageViews.size());
            for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
            {
                std::array<VkImageView, 1> attachments = {
                    m_swapChainImageViews[i],
                };

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = m_renderPassImGui;
                framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                framebufferInfo.pAttachments = attachments.data();
                framebufferInfo.width = m_swapChainExtent.width;
                framebufferInfo.height = m_swapChainExtent.height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_imGuiFrameBuffers[i]) != VK_SUCCESS) 
                {
                    throw std::runtime_error("failed to create imgui framebuffer!");
                }
            }
        }
    }
#pragma endregion
#pragma region commandPool
    void createCommandPool()
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

        if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }
#pragma endregion

#pragma region commandBuffer
    
    void createCommandBuffers() 
    {
        m_commandBuffers.resize(m_swapChainFramebuffers.size());
        m_imGuiCommandBuffers.resize(m_swapChainFramebuffers.size());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) m_commandBuffers.size();

        if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers");
        }

        if (vkAllocateCommandBuffers(m_device, &allocInfo, m_imGuiCommandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers");
        }

    }
#pragma endregion
#pragma region draw
    void drawFrame(ImDrawData* imGuiDrawData) 
    {
        vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
    
        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        updateUniformBuffer(imageIndex);
        generateCommandBuffer(imageIndex);
        generateImGuiCommandBuffer(imageIndex, imGuiDrawData);

        std::array<VkCommandBuffer, 2> commandBuffers = {m_commandBuffers[imageIndex], m_imGuiCommandBuffers[imageIndex]};
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 2;
        submitInfo.pCommandBuffers = commandBuffers.data();
        VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = {m_swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional
        vkQueuePresentKHR(m_presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized) 
        {
            m_framebufferResized = false;
            recreateSwapChain();
        } 
        else if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    }

#pragma endregion

#pragma region sync
    void createSyncObjects() 
    {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
            {

                throw std::runtime_error("failed to create sync objects!");
            }
        }
    }
#pragma endregion

    void initVulkan()
    {
        createInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDescriptorSetLayout();
        createPipelineLayout();
        createGraphicsPipeline();
        createDepthResources();
        createFramebuffers();
        createCommandPool();
        m_tfDevice.physicsDevice = m_physicalDevice;
        m_tfDevice.cmdPool = m_commandPool;
        m_tfDevice.logicalDevice = m_device;
        m_tfDevice.graphicsQueue = m_graphicsQueue;
        //createTextureImage();
        //createTextureImageView();
        //m_textureSampler = CreateTextureSampler(m_physicalDevice, m_device,m_mipLevels);
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();
        m_gltfModel = new GltfModel(m_tfDevice,"../../res/model/FlightHelmet/FlightHelmet.gltf",5.0f);
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();
        
        
    }

    void initImGui()
    {
        

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();


        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);
        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(m_window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = m_instance;
        init_info.PhysicalDevice = m_physicalDevice;
        init_info.Device = m_device;
        init_info.QueueFamily = queueFamilyIndices.graphicsFamily.value();
        init_info.Queue = m_graphicsQueue;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = m_descriptorPoolImGui;
        init_info.Allocator = VK_NULL_HANDLE;
        init_info.MinImageCount = 2;
        init_info.ImageCount = m_swapChainFramebuffers.size();
        init_info.CheckVkResultFn = check_vk_result;
        
        m_mainWindowData.RenderPass = m_renderPassImGui;
        ImGui_ImplVulkan_Init(&init_info, m_mainWindowData.RenderPass);

        VkCommandBuffer command_buffer = beginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
        endSingleTimeCommands(command_buffer);



    }

    void cleanupSwapChain() 
    {
        for (size_t i = 0; i < m_swapChainFramebuffers.size(); i++) 
        {
            vkDestroyFramebuffer(m_device, m_swapChainFramebuffers[i], nullptr);
            vkDestroyFramebuffer(m_device, m_imGuiFrameBuffers[i], nullptr);
            vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr);
            vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr);
        }
        vkDestroyImageView(m_device, m_depthImageView, nullptr);
        vkDestroyImage(m_device, m_depthImage, nullptr);
        vkFreeMemory(m_device, m_depthImageMemory, nullptr);

        vkDestroyDescriptorPool(m_device, m_descriptorPoolImGui, nullptr);
        vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);

        vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_imGuiCommandBuffers.size()), m_imGuiCommandBuffers.data());
        vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());

        vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
        vkDestroyRenderPass(m_device, m_renderPassImGui, nullptr);
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);

        for (size_t i = 0; i < m_swapChainImageViews.size(); i++) 
        {
            vkDestroyImageView(m_device, m_swapChainImageViews[i], nullptr);
        }

        vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
    }

    void recreateSwapChain()
    {

        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        while (width == 0 || height == 0) 
        {
            glfwGetFramebufferSize(m_window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_device);

        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createDepthResources();
        createFramebuffers();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
    }

    void mainLoop()
    {
        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        while (!glfwWindowShouldClose(m_window)) 
        {
            
            glfwPollEvents();
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
            {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);
            ImGui::SliderFloat("CameraAngle", &m_cameraAngle, 0.0f, 89.0f);
            ImGui::SliderFloat("CameraDistance", &m_cameraDistance, 0.0f, 1000.0f);
            
            ImGui::SliderFloat("CameraOffsetX",&m_cameraOffsetX,-100,100);
            ImGui::SliderFloat("light intensity", &m_lightIntensity, 0.0f, 100.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("light color", (float*)&m_lightColor); // Edit 3 floats representing a color
            ImGui::SliderFloat("light_x", &m_lightDir.x, -1.0f, 1.0f);
            ImGui::SliderFloat("light_y", &m_lightDir.y, -1.0f, 1.0f);
            ImGui::SliderFloat("light_z", &m_lightDir.z, -1.0f, 1.0f);
            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
            drawFrame(draw_data);
            // 防止显卡啸叫
            Sleep(10);// https://en.wikipedia.org/wiki/Electromagnetically_induced_acoustic_noise
        }
        vkDeviceWaitIdle(m_device);
    }

    void cleanup()
    {
        
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        m_world.Shutdown();
        //m_terrainSystem.CleanUp();
        m_skySystem.CleanUp();
        cleanupSwapChain();

        delete(m_gltfModel);

        //vkDestroySampler(m_device, m_textureSampler, nullptr);
        //vkDestroyImageView(m_device, m_textureImageView, nullptr);

        //vkDestroyImage(m_device, m_textureImage, nullptr);
        //vkFreeMemory(m_device, m_textureImageMemory, nullptr);

        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayoutMatrices, nullptr);
        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayoutTextures, nullptr);
        vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
        vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
        vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
        vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(m_device, m_commandPool, nullptr);


        vkDestroyDevice(m_device, nullptr);
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);

        glfwDestroyWindow(m_window);

        glfwTerminate();
    }
};

int main() 
{
    // std::cout<<"Start Engine"<<std::endl;
    // fs::path resPath = Vfs::GetResPath();
    // resPath += "\\model\\sphere\\sphere.obj";
    // ObjModel a = ObjModel(resPath);
    VulkanRendererApp app;
    try
    {
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    std::cout<<"Shutdown Engine"<<std::endl;
    return EXIT_SUCCESS;
    
}