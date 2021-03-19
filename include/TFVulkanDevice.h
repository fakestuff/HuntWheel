#pragma once
#include <vulkan/vulkan.hpp>
namespace TF
{
    
    class TFVkGfxDevice
    {
    public:
        VkPhysicalDevice physicsDevice;
        VkDevice logicalDevice;
        VkQueue graphicsQueue;
        VkCommandPool cmdPool;
        TFVkGfxDevice() {}
        ~TFVkGfxDevice() {}
        TFVkGfxDevice(const TFVkGfxDevice&) = delete;
        TFVkGfxDevice(TFVkGfxDevice&&) = delete;
        TFVkGfxDevice& operator=(const TFVkGfxDevice&) = delete;
        TFVkGfxDevice& operator=(TFVkGfxDevice&&) = delete;
    inline static TFVkGfxDevice& Get() 
    {
        static TFVkGfxDevice instance;
        return instance;
    }
    };
    
}