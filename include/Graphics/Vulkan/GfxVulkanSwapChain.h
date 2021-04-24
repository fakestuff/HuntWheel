#pragma once
#include <vulkan/vulkan.hpp>
namespace TF {
namespace GFX {
    class GfxVkDevice;
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct SwapChainBuffer 
    {
        VkImage image;
        VkImageView view;
    };

    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

    class GfxVkSwapChain
    {
    public:
        GfxVkDevice* m_gfxVkDevice;
        VkFormat m_colorFormat;
        VkColorSpaceKHR m_colorSpace;
        VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
        uint32_t m_imageCount;
        std::vector<VkImage> m_images;
        std::vector<SwapChainBuffer> m_buffers;
        uint32_t m_queueNodeIndex = UINT32_MAX;
        GfxVkSwapChain(GfxVkDevice* gfxVkDevice);
        ~GfxVkSwapChain();

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);


        // surface is created in gfx device
        void InitSurface();
        VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);
        VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
        void Cleanup();
    };
}
}