#pragma once
#include <vulkan/vulkan.hpp>
uint32_t findMemoryType(VkPhysicalDevice physicalDevice,uint32_t typeFilter, VkMemoryPropertyFlags properties);
void createBuffer(VkPhysicalDevice physicalDevice,VkDevice device,VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
void EndSingleTimeCommands(VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool,VkCommandBuffer commandBuffer);
void CopyBuffer(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
