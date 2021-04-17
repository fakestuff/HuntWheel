#pragma once

#include <vector>

#include <vulkan/vulkan.hpp>
#include <VulkanHelper.h>
namespace TF {
namespace GFX {
	class GfxVkBuffer
	{
	public:
		VkDevice m_device;
		VkBuffer m_buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_memory = VK_NULL_HANDLE;
		VkDescriptorBufferInfo m_descriptor;
		VkDeviceSize m_size = 0;
		VkDeviceSize m_alignment = 0;
		void* m_mapped = nullptr;
		/** @brief Usage flags to be filled by external source at buffer creation (to query at some later point) */
		VkBufferUsageFlags m_usageFlags;
		/** @brief Memory property flags to be filled by external source at buffer creation (to query at some later point) */
		VkMemoryPropertyFlags m_memoryPropertyFlags;
		VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void Unmap();
		VkResult Bind(VkDeviceSize offset = 0);
		void SetupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void CopyTo(void* data, VkDeviceSize size);
		VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void Destroy();
	};
}
}