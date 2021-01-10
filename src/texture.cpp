#include "texture.h"
#include <VulkanBufferHelper.h>
namespace TF
{
    Texture::Texture(int width, int height, CPUTextureFormat cpuFormat, unsigned char* pixels)
    {
        m_width = width;
        m_height = height;
        CPUFormat = cpuFormat;
        
        switch (cpuFormat)
        {
            case CPUTextureFormat::R8:
                m_bytePerPixel = 1;
                break;
            case CPUTextureFormat::R8G8:
                m_bytePerPixel = 2;
                break;
            case CPUTextureFormat::R8G8B8:
                m_bytePerPixel = 3;
                break;
            case CPUTextureFormat::R8G8B8A8:
                m_bytePerPixel = 4;
                break;
            default:
                m_bytePerPixel = 4;
        }
        Pixels.reserve(m_width*m_height*m_bytePerPixel);
        memcpy(Pixels.data(), pixels, m_width*m_height*m_bytePerPixel);
    }

    void Texture::UpdateDescriptor()
    {
        m_descriptor.sampler = m_sampler;
        m_descriptor.imageView = m_view;
        m_descriptor.imageLayout = m_imageLayout;

    }

    void Texture::Destroy()
    {
        vkDestroyImageView(m_device->logicalDevice, m_view, nullptr);
        vkDestroyImage(m_device->logicalDevice, m_image, nullptr);
        if (m_sampler)
        {
            vkDestroySampler(m_device->logicalDevice, m_sampler, nullptr);
        }
        vkFreeMemory(m_device->logicalDevice, m_deviceMemory, nullptr);
    }

    void Texture2D::UpdateDescriptor()
    {
        m_descriptor.sampler = m_sampler;
        m_descriptor.imageView = m_view;
        m_descriptor.imageLayout = m_imageLayout;
    }


    void Texture2D::FromBuffer(void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth, uint32_t texHeight, TFVulkanDevice *device, VkQueue copyQueue, VkFilter filter, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout, TextureUsage usage)
	{
		assert(buffer);

        m_device = device;
        m_imageLayout = imageLayout;
        m_usage = usage;
        m_width = texWidth;
        m_height = texHeight;
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        CreateBuffer(m_device->physicsDevice,m_device->logicalDevice,imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);
        void* data;
        vkMapMemory(m_device->logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, buffer, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_device->logicalDevice, stagingBufferMemory);

        CreateImage(m_device->physicsDevice, m_device->logicalDevice, texWidth, texHeight, m_mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_deviceMemory);
        
        TransitionImageLayout(m_device->logicalDevice,m_device->graphicsQueue,m_device->cmdPool, m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,m_mipLevels);
        CopyBufferToImage(m_device->logicalDevice,m_device->graphicsQueue,m_device->cmdPool,stagingBuffer, m_image, texWidth, texHeight);
        
        //transitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,m_mipLevels);
        GenerateMipmaps(m_device->physicsDevice, m_device->logicalDevice, m_device->graphicsQueue, m_device->cmdPool, m_image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, m_mipLevels);
        vkDestroyBuffer(m_device->logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(m_device->logicalDevice, stagingBufferMemory, nullptr);

        m_sampler = CreateTextureSampler( m_device->physicsDevice, m_device->logicalDevice, m_mipLevels);
		m_view = CreateImageView(m_device->logicalDevice, m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels);

		// // Update descriptor image info member that can be used for setting up descriptor sets
		UpdateDescriptor();
	}

    void Texture2D::Destroy()
    {
        vkDestroyImageView(m_device->logicalDevice, m_view, nullptr);
        vkDestroyImage(m_device->logicalDevice, m_image, nullptr);
        if (m_sampler)
        {
            vkDestroySampler(m_device->logicalDevice, m_sampler, nullptr);
        }
        vkFreeMemory(m_device->logicalDevice, m_deviceMemory, nullptr);
    }

}