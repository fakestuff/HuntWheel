#pragma once
#include <vulkan/vulkan.hpp>
uint32_t findMemoryType(VkPhysicalDevice physicalDevice,uint32_t typeFilter, VkMemoryPropertyFlags properties);
void CreateBuffer(VkPhysicalDevice physicalDevice,VkDevice device,VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
void EndSingleTimeCommands(VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool,VkCommandBuffer commandBuffer);
void CopyBuffer(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void CreateImage(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
void TransitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
void CopyBufferToImage(VkDevice device, VkQueue queue, VkCommandPool commandPool,VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
void GenerateMipmaps(VkPhysicalDevice physicalDevice,VkDevice device, VkQueue queue, VkCommandPool commandPool,VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
VkImageView CreateImageView(VkDevice device,VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
VkSampler CreateTextureSampler(VkPhysicalDevice physicalDevice, VkDevice device, int mipLevel);