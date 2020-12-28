#pragma once
#include <vulkan/vulkan.hpp>

namespace TF
{
    struct TFVulkanDevice
    {
        VkPhysicalDevice physicsDevice;
        VkDevice logicalDevice;
        VkQueue graphicsQueue;
        VkCommandPool cmdPool;
    };
}