/*
* Vulkan buffer class
*
* Encapsulates a Vulkan buffer
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <Graphics/Vulkan/VulkanBuffer.h>

namespace TF {
namespace GFX {
	/**
	* Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
	*
	* @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
	* @param offset (Optional) Byte offset from beginning
	*
	* @return VkResult of the buffer mapping call
	*/
	VkResult GfxVkBuffer::Map(VkDeviceSize size, VkDeviceSize offset)
	{
		return vkMapMemory(m_device, m_memory, offset, size, 0, &m_mapped);
	}

	/**
	* Unmap a mapped memory range
	*
	* @note Does not return a result as vkUnmapMemory can't fail
	*/
	void GfxVkBuffer::Unmap()
	{
		if (m_mapped)
		{
			vkUnmapMemory(m_device, m_memory);
			m_mapped = nullptr;
		}
	}

	/**
	* Attach the allocated memory block to the buffer
	*
	* @param offset (Optional) Byte offset (from the beginning) for the memory region to bind
	*
	* @return VkResult of the bindBufferMemory call
	*/
	VkResult GfxVkBuffer::Bind(VkDeviceSize offset)
	{
		return vkBindBufferMemory(m_device, m_buffer, m_memory, offset);
	}

	/**
	* Setup the default descriptor for this buffer
	*
	* @param size (Optional) Size of the memory range of the descriptor
	* @param offset (Optional) Byte offset from beginning
	*
	*/
	void GfxVkBuffer::SetupDescriptor(VkDeviceSize size, VkDeviceSize offset)
	{
		m_descriptor.offset = offset;
		m_descriptor.buffer = m_buffer;
		m_descriptor.range = size;
	}

	/**
	* Copies the specified data to the mapped buffer
	*
	* @param data Pointer to the data to copy
	* @param size Size of the data to copy in machine units
	*
	*/
	void GfxVkBuffer::CopyTo(void* data, VkDeviceSize size)
	{
		assert(m_mapped);
		memcpy(m_mapped, data, size);
	}

	/**
	* Flush a memory range of the buffer to make it visible to the device
	*
	* @note Only required for non-coherent memory
	*
	* @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the complete buffer range.
	* @param offset (Optional) Byte offset from beginning
	*
	* @return VkResult of the flush call
	*/
	VkResult GfxVkBuffer::Flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = m_memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(m_device, 1, &mappedRange);
	}

	/**
	* Invalidate a memory range of the buffer to make it visible to the host
	*
	* @note Only required for non-coherent memory
	*
	* @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate the complete buffer range.
	* @param offset (Optional) Byte offset from beginning
	*
	* @return VkResult of the invalidate call
	*/
	VkResult GfxVkBuffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = m_memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(m_device, 1, &mappedRange);
	}
	/**
	* Release all Vulkan resources held by this buffer
	*/
	void GfxVkBuffer::Destroy()
	{
		if (m_buffer)
		{
			vkDestroyBuffer(m_device, m_buffer, nullptr);
		}
		if (m_memory)
		{
			vkFreeMemory(m_device, m_memory, nullptr);
		}
	}
}
}