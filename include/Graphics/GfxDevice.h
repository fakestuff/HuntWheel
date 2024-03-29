#pragma once

#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>

#include <VulkanHelper.h>
#include <Graphics/Vulkan/VulkanBuffer.h>
struct GLFWwindow;
namespace TF {
namespace GFX {
	
	class GfxVkDevice
	{
	public:
		/** @brief vk instance representation */
		VkInstance m_instance;
		/** @brief vk surface for window system */
		VkSurfaceKHR m_surface;
		
		/** @brief Physical device representation */
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		/** @brief Logical device representation (application's view of the device) */
		VkDevice m_logicalDevice = VK_NULL_HANDLE;
		/** @brief Properties of the physical device including limits that the application can check against */
		VkPhysicalDeviceProperties m_properties;
		/** @brief Features of the physical device that an application can use to check if a feature is supported */
		VkPhysicalDeviceFeatures m_features;
		/** @brief Features that have been enabled for use on the physical device */
		VkPhysicalDeviceFeatures m_enabledFeatures;
		/** @brief Memory types and heaps of the physical device */
		VkPhysicalDeviceMemoryProperties m_memoryProperties;
		/** @brief Queue family properties of the physical device */
		std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
		/** @brief List of extensions supported by the device */
		std::vector<std::string> m_supportedExtensions;
		/** @brief Default command pool for the graphics queue family index */
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		/** @brief Set to true when the debug marker extension is detected */
		bool m_enableDebugMarkers = false;
		/** @brief Contains queue family indices */

		GLFWwindow* m_window;
		
		QueueFamilyIndices m_queueFamilyIndices;


		

		explicit GfxVkDevice(GLFWwindow* m_window);
		~GfxVkDevice();
		bool			CheckValidationLayerSupport();
		void			CreateInstance(bool enableValidationLayers);
		
		bool			CheckDeviceExtensionSupport(VkPhysicalDevice device);
		bool			IsPhysicalDeviceSuitable(VkPhysicalDevice device);
		void			ChoosePhysicalDevice();

		void			CreateSurface();
		void			GetBackBufferSize(int* width, int* height);
		void			PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		std::vector<const char*> GetRequiredExtensions(bool enableValidationLayers);
		uint32_t        GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr) const;
		uint32_t        GetQueueFamilyIndex(VkQueueFlagBits queueFlags) const;
		VkResult        CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, void* pNextChain, bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
		VkResult        CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr);
		VkResult        CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, GfxVkBuffer* buffer, VkDeviceSize size, void* data = nullptr);
		void            CopyBuffer(GfxVkBuffer* src, GfxVkBuffer* dst, VkQueue queue, VkBufferCopy* copyRegion = nullptr);
		VkCommandPool   CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false);
		VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin = false);
		void            FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true);
		void            FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
		bool            ExtensionSupported(std::string extension);
		VkFormat        GetSupportedDepthFormat(bool checkSamplingSupport);
	};
}
}