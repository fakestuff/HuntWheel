#pragma once
#include <memory>
#include <vector>
#include <TFVulkanDevice.h>
namespace TF // TF stand for touch fish :)
{
    enum CPUTextureFormat
    {
        R8,
        R8G8,
        R8G8B8,
        R8G8B8A8,
        FLOAT16X3,
        FLOAT16X4,
    };
    class Texture // TODO: Refractor this later, temp hack for now
    {
        public:
            Texture(int width, int height, CPUTextureFormat cpuFormat, unsigned char* pixels);
            void UpdateDescriptor();
            void Destroy();

            CPUTextureFormat CPUFormat;
            
            
            int m_width;
            int m_height;
            int Width() {return m_width;}
            int Height() {return m_height;}
            int m_bytePerPixel; // how many bytes per texel
            int m_mipLevels;
            std::vector<unsigned char> Pixels;

            TFVulkanDevice* m_device;
            VkImage m_image;
            VkImageLayout m_imageLayout;
            VkDeviceMemory m_deviceMemory;
            VkImageView m_view;
            VkDescriptorImageInfo m_descriptor;
            VkSampler m_sampler;

            

        private:
    };

    class Texture2D 
    {
    public:
        int m_width;
        int m_height;
        int Width() {return m_width;}
        int Height() {return m_height;}
        int m_bytePerPixel; // how many bytes per texel
        int m_mipLevels;
        std::vector<unsigned char> Pixels;

        TFVulkanDevice* m_device;
        VkImage m_image;
        VkImageLayout m_imageLayout;
        VkDeviceMemory m_deviceMemory;
        VkImageView m_view;
        VkDescriptorImageInfo m_descriptor;
        VkSampler m_sampler;
        void UpdateDescriptor();
        void Destroy();
        void FromBuffer(
            void *              buffer,
            VkDeviceSize        buffersize,
            VkFormat            format,
            uint32_t            texWidth,
            uint32_t            texHeight,
            TFVulkanDevice     *device,
            VkQueue             copyQueue,
            VkFilter            filter          = VK_FILTER_LINEAR,
            VkImageUsageFlags   imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
            VkImageLayout       imageLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    };


} // namespace TF

