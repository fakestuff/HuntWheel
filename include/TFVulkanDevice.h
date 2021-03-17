#pragma once
#include <vulkan/vulkan.hpp>
namespace TF
{
    
    struct TFVkGfxDevice
    {
        VkPhysicalDevice physicsDevice;
        VkDevice logicalDevice;
        VkQueue graphicsQueue;
        VkCommandPool cmdPool;
    };
    static TFVkGfxDevice s_tfVulkanDevice;
}