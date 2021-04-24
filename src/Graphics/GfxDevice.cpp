/*
* GFX Vulkan device class
* 
* Most code are coming from Sascha Willems' vulkan repo
*
* Encapsulates a physical Vulkan device and its logical representation
*
* Copyright (C) by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <Graphics/GfxDevice.h>
#include <Graphics/Vulkan/GfxVulkanSwapChain.h>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <set>

namespace TF {
namespace GFX {

	const std::vector<const char*> GValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> GDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
	};

	static VKAPI_ATTR VkBool32 VKAPI_CALL SDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
	GfxVkDevice::GfxVkDevice(GLFWwindow* window) : m_window(window)
	{
		CreateInstance(true);
		CreateSurface();
		ChoosePhysicalDevice();
		if (this->m_physicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Could not find physical device device: \n");
		}
		
		// Store Properties features, limits and properties of the physical device for later use
		// Device properties also contain limits and sparse properties
		vkGetPhysicalDeviceProperties(this->m_physicalDevice, &m_properties);
		// Features should be checked by the examples before using them
		vkGetPhysicalDeviceFeatures(this->m_physicalDevice, &m_features);
		// Memory properties are used regularly for creating all kinds of buffers
		vkGetPhysicalDeviceMemoryProperties(this->m_physicalDevice, &m_memoryProperties);
		// Queue family properties, used for setting up requested queues upon device creation
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(this->m_physicalDevice, &queueFamilyCount, nullptr);
		assert(queueFamilyCount > 0);
		m_queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(this->m_physicalDevice, &queueFamilyCount, m_queueFamilyProperties.data());

		// Get list of supported extensions
		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties(this->m_physicalDevice, nullptr, &extCount, nullptr);
		if (extCount > 0)
		{
			std::vector<VkExtensionProperties> extensions(extCount);
			if (vkEnumerateDeviceExtensionProperties(this->m_physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
			{
				for (auto ext : extensions)
				{
					m_supportedExtensions.push_back(ext.extensionName);
				}
			}
		}
		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		void* deviceCreatepNextChain = nullptr;

		VkResult res = CreateLogicalDevice(deviceFeatures, GDeviceExtensions, deviceCreatepNextChain);
		if (res != VK_SUCCESS) {
			throw std::runtime_error("Could not create Vulkan device: \n"+VkErrorString(res));
		}
	}
	GfxVkDevice::~GfxVkDevice()
	{
		if (m_commandPool)
		{
			vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
		}
		if (m_logicalDevice)
		{
			vkDestroyDevice(m_logicalDevice, nullptr);
		}
		if (m_surface)
		{
			vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		}
		if (m_instance)
		{
			vkDestroyInstance(m_instance, nullptr);
		}
	}

	bool GfxVkDevice::CheckValidationLayerSupport()
	{

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : GValidationLayers)
		{
			// iterator through required layer
			// the loop in the vulkan tutorial is a little bit wiered

			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}
			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	void GfxVkDevice::CreateInstance(bool enableValidationLayers)
	{
		if (enableValidationLayers && !CheckValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "VulkanRenderer";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = GetRequiredExtensions(enableValidationLayers);
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(GValidationLayers.size());
			createInfo.ppEnabledLayerNames = GValidationLayers.data();
			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
			// enable best practice
			VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT };
			VkValidationFeaturesEXT features = {};
			features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
			features.enabledValidationFeatureCount = 1;
			features.pEnabledValidationFeatures = enables;
			debugCreateInfo.pNext = &features;
		}
		else
		{
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}

		uint32_t totalExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &totalExtensionCount, nullptr);
		std::vector<VkExtensionProperties> totalExtensions(totalExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &totalExtensionCount, totalExtensions.data());
		std::cout << "available extensions:" << std::endl;


		for (const auto& extension : totalExtensions) {
			std::cout << '\t' << extension.extensionName << std::endl;
		}
	}



	bool GfxVkDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(GDeviceExtensions.begin(), GDeviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}
		return requiredExtensions.empty();
	}

	

	bool GfxVkDevice::IsPhysicalDeviceSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		bool extensionsSupported = CheckDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, m_surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		// most case there is only one discrete gpu, so this should be fine for now.
		return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			&& extensionsSupported
			&& swapChainAdequate
			&& deviceFeatures.samplerAnisotropy;
	}

	void GfxVkDevice::ChoosePhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
		
		
		for (const auto& device : devices)
		{
			if (IsPhysicalDeviceSuitable(device))
			{
				m_physicalDevice = device;
				break;
			}
		}
		if (m_physicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
	}

	void GfxVkDevice::GetBackBufferSize(int* width, int* height)
	{
		glfwGetFramebufferSize(m_window, width, height);
	}

	void GfxVkDevice::CreateSurface()
	{
		if (glfwCreateWindowSurface(
			m_instance,
			m_window,
			nullptr,
			&m_surface)
			!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface");
		}
	}


	void GfxVkDevice::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT || VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT 
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = SDebugCallback;
	}

	std::vector<const char*> GfxVkDevice::GetRequiredExtensions(bool enableValidationLayers)
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	uint32_t GfxVkDevice::GetMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const
	{
		for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((m_memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					if (memTypeFound)
					{
						*memTypeFound = true;
					}
					return i;
				}
			}
			typeBits >>= 1;
		}

		if (memTypeFound)
		{
			*memTypeFound = false;
			return 0;
		}
		else
		{
			throw std::runtime_error("Could not find a matching memory type");
		}
	}
	uint32_t GfxVkDevice::GetQueueFamilyIndex(VkQueueFlagBits queueFlags) const
	{
		// Dedicated queue for compute
		// Try to find a queue family index that supports compute but not graphics
		if (queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++)
			{
				if ((m_queueFamilyProperties[i].queueFlags & queueFlags) && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				{
					return i;
				}
			}
		}

		// Dedicated queue for transfer
		// Try to find a queue family index that supports transfer but not graphics and compute
		if (queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++)
			{
				if ((m_queueFamilyProperties[i].queueFlags & queueFlags) && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
				{
					return i;
				}
			}
		}

		if (queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++)
			{
				if ((m_queueFamilyProperties[i].queueFlags & queueFlags))
				{
					
					VkBool32 presentSupport = false;
					vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &presentSupport);
					if (presentSupport)
					{
						return i;
					}
					
				}
			}
		}

		// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++)
		{
			if (m_queueFamilyProperties[i].queueFlags & queueFlags)
			{
				return i;
			}
		}

		throw std::runtime_error("Could not find a matching queue family index");
	}

	/**
	* Create the logical device based on the assigned physical device, also gets default queue family indices
	*
	* @param enabledFeatures Can be used to enable certain features upon device creation
	* @param pNextChain Optional chain of pointer to extension structures
	* @param useSwapChain Set to false for headless rendering to omit the swapchain device extensions
	* @param requestedQueueTypes Bit flags specifying the queue types to be requested from the device
	*
	* @return VkResult of the device creation call
	*/
	VkResult GfxVkDevice::CreateLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, void* pNextChain, bool useSwapChain, VkQueueFlags requestedQueueTypes)
	{
		// Desired queues need to be requested upon logical device creation
		// Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
		// requests different queue types

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

		// Get queue family indices for the requested queue family types
		// Note that the indices may overlap depending on the implementation

		const float defaultQueuePriority(0.0f);

		// Graphics queue
		if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
		{
			m_queueFamilyIndices.m_graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = m_queueFamilyIndices.m_graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
		else
		{
			m_queueFamilyIndices.m_graphics = VK_NULL_HANDLE;
			throw std::runtime_error("Could not find a queue support both graphics and present");
		}

		// Dedicated compute queue
		if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			m_queueFamilyIndices.m_compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			if (m_queueFamilyIndices.m_compute != m_queueFamilyIndices.m_graphics)
			{
				// If compute family index differs, we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = m_queueFamilyIndices.m_compute;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			m_queueFamilyIndices.m_compute = m_queueFamilyIndices.m_graphics;
		}

		// Dedicated transfer queue
		if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		{
			m_queueFamilyIndices.m_transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
			if ((m_queueFamilyIndices.m_transfer != m_queueFamilyIndices.m_graphics) && (m_queueFamilyIndices.m_transfer != m_queueFamilyIndices.m_compute))
			{
				// If compute family index differs, we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = m_queueFamilyIndices.m_transfer;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			m_queueFamilyIndices.m_transfer = m_queueFamilyIndices.m_graphics;
		}

		// Create the logical device representation
		std::vector<const char*> deviceExtensions(enabledExtensions);
		if (useSwapChain)
		{
			// If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
			deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

		// If a pNext(Chain) has been passed, we need to add it to the device creation info
		VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
		if (pNextChain) {
			physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			physicalDeviceFeatures2.features = enabledFeatures;
			physicalDeviceFeatures2.pNext = pNextChain;
			deviceCreateInfo.pEnabledFeatures = nullptr;
			deviceCreateInfo.pNext = &physicalDeviceFeatures2;
		}

		// Enable the debug marker extension if it is present (likely meaning a debugging tool is present)
		if (ExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
		{
			deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
			m_enableDebugMarkers = true;
		}

		if (deviceExtensions.size() > 0)
		{
			for (const char* enabledExtension : deviceExtensions)
			{
				if (!ExtensionSupported(enabledExtension)) {
					std::cerr << "Enabled device extension \"" << enabledExtension << "\" is not present at device level\n";
				}
			}

			deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

		this->m_enabledFeatures = enabledFeatures;

		VkResult result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice);
		if (result != VK_SUCCESS)
		{
			return result;
		}

		// Create a default command pool for graphics command buffers
		std::cout << "Created logical device" << std::endl;
		m_commandPool = CreateCommandPool(m_queueFamilyIndices.m_graphics);

		return result;
	}

	/**
	* Create a buffer on the device
	*
	* @param usageFlags Usage flag bit mask for the buffer (i.e. index, vertex, uniform buffer)
	* @param memoryPropertyFlags Memory properties for this buffer (i.e. device local, host visible, coherent)
	* @param size Size of the buffer in byes
	* @param buffer Pointer to the buffer handle acquired by the function
	* @param memory Pointer to the memory handle acquired by the function
	* @param data Pointer to the data that should be copied to the buffer after creation (optional, if not set, no data is copied over)
	*
	* @return VK_SUCCESS if buffer handle and memory have been created and (optionally passed) data has been copied
	*/
	VkResult GfxVkDevice::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data)
	{
		// Create the buffer handle
		VkBufferCreateInfo bufferCreateInfo = VkInitializers::BufferCreateInfo(usageFlags, size);
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK_RESULT(vkCreateBuffer(m_logicalDevice, &bufferCreateInfo, nullptr, buffer));

		// Create the memory backing up the buffer handle
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc = VkInitializers::MemoryAllocateInfo();
		vkGetBufferMemoryRequirements(m_logicalDevice, *buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		// Find a memory type index that fits the properties of the buffer
		memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
		// If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
		VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
		if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
			allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
			allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
			memAlloc.pNext = &allocFlagsInfo;
		}
		VK_CHECK_RESULT(vkAllocateMemory(m_logicalDevice, &memAlloc, nullptr, memory));

		// If a pointer to the buffer data has been passed, map the buffer and copy over the data
		if (data != nullptr)
		{
			void* mapped;
			VK_CHECK_RESULT(vkMapMemory(m_logicalDevice, *memory, 0, size, 0, &mapped));
			memcpy(mapped, data, size);
			// If host coherency hasn't been requested, do a manual flush to make writes visible
			if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			{
				VkMappedMemoryRange mappedRange = VkInitializers::MappedMemoryRange();
				mappedRange.memory = *memory;
				mappedRange.offset = 0;
				mappedRange.size = size;
				vkFlushMappedMemoryRanges(m_logicalDevice, 1, &mappedRange);
			}
			vkUnmapMemory(m_logicalDevice, *memory);
		}

		// Attach the memory to the buffer object
		VK_CHECK_RESULT(vkBindBufferMemory(m_logicalDevice, *buffer, *memory, 0));

		return VK_SUCCESS;
	}

	VkResult GfxVkDevice::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, GfxVkBuffer* buffer, VkDeviceSize size, void* data)
	{
		buffer->m_device = m_logicalDevice;

		// Create the buffer handle
		VkBufferCreateInfo bufferCreateInfo = VkInitializers::BufferCreateInfo(usageFlags, size);
		VK_CHECK_RESULT(vkCreateBuffer(m_logicalDevice, &bufferCreateInfo, nullptr, &buffer->m_buffer));

		// Create the memory backing up the buffer handle
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc = VkInitializers::MemoryAllocateInfo();
		vkGetBufferMemoryRequirements(m_logicalDevice, buffer->m_buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		// Find a memory type index that fits the properties of the buffer
		memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
		// If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
		VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
		if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
			allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
			allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
			memAlloc.pNext = &allocFlagsInfo;
		}
		VK_CHECK_RESULT(vkAllocateMemory(m_logicalDevice, &memAlloc, nullptr, &buffer->m_memory));

		buffer->m_alignment = memReqs.alignment;
		buffer->m_size = size;
		buffer->m_usageFlags = usageFlags;
		buffer->m_memoryPropertyFlags = memoryPropertyFlags;

		// If a pointer to the buffer data has been passed, map the buffer and copy over the data
		if (data != nullptr)
		{
			VK_CHECK_RESULT(buffer->Map());
			memcpy(buffer->m_mapped, data, size);
			if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
				buffer->Flush();

			buffer->Unmap();
		}

		// Initialize a default descriptor that covers the whole buffer size
		buffer->SetupDescriptor();

		// Attach the memory to the buffer object
		return buffer->Bind();
	}


	/**
	* Copy buffer data from src to dst using VkCmdCopyBuffer
	*
	* @param src Pointer to the source buffer to copy from
	* @param dst Pointer to the destination buffer to copy to
	* @param queue Pointer
	* @param copyRegion (Optional) Pointer to a copy region, if NULL, the whole buffer is copied
	*
	* @note Source and destination pointers must have the appropriate transfer usage flags set (TRANSFER_SRC / TRANSFER_DST)
	*/
	void GfxVkDevice::CopyBuffer(GfxVkBuffer* src, GfxVkBuffer* dst, VkQueue queue, VkBufferCopy* copyRegion)
	{
		assert(dst->m_size <= src->m_size);
		assert(src->m_buffer);
		VkCommandBuffer copyCmd = CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		VkBufferCopy bufferCopy{};
		if (copyRegion == nullptr)
		{
			bufferCopy.size = src->m_size;
		}
		else
		{
			bufferCopy = *copyRegion;
		}

		vkCmdCopyBuffer(copyCmd, src->m_buffer, dst->m_buffer, 1, &bufferCopy);

		FlushCommandBuffer(copyCmd, queue);
	}

	/**
	* Create a command pool for allocation command buffers from
	*
	* @param queueFamilyIndex Family index of the queue to create the command pool for
	* @param createFlags (Optional) Command pool creation flags (Defaults to VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
	*
	* @note Command buffers allocated from the created pool can only be submitted to a queue with the same family index
	*
	* @return A handle to the created command buffer
	*/
	VkCommandPool GfxVkDevice::CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
		cmdPoolInfo.flags = createFlags;
		VkCommandPool cmdPool;
		VK_CHECK_RESULT(vkCreateCommandPool(m_logicalDevice, &cmdPoolInfo, nullptr, &cmdPool));
		std::cout << "Created cmd pool with queue family index: " << queueFamilyIndex << std::endl;
		return cmdPool;
	}
	/**
	* Allocate a command buffer from the command pool
	*
	* @param level Level of the new command buffer (primary or secondary)
	* @param pool Command pool from which the command buffer will be allocated
	* @param (Optional) begin If true, recording on the new command buffer will be started (vkBeginCommandBuffer) (Defaults to false)
	*
	* @return A handle to the allocated command buffer
	*/
	VkCommandBuffer GfxVkDevice::CreateCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin)
	{
		VkCommandBufferAllocateInfo cmdBufAllocateInfo = VkInitializers::CommandBufferAllocateInfo(pool, level, 1);
		VkCommandBuffer cmdBuffer;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_logicalDevice, &cmdBufAllocateInfo, &cmdBuffer));
		// If requested, also start recording for the new command buffer
		if (begin)
		{
			VkCommandBufferBeginInfo cmdBufInfo = VkInitializers::CommandBufferBeginInfo();
			VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
		}
		return cmdBuffer;
	}

	VkCommandBuffer GfxVkDevice::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
	{
		return CreateCommandBuffer(level, m_commandPool, begin);
	}

	/**
	* Finish command buffer recording and submit it to a queue
	*
	* @param commandBuffer Command buffer to flush
	* @param queue Queue to submit the command buffer to
	* @param pool Command pool on which the command buffer has been created
	* @param free (Optional) Free the command buffer once it has been submitted (Defaults to true)
	*
	* @note The queue that the command buffer is submitted to must be from the same family index as the pool it was allocated from
	* @note Uses a fence to ensure command buffer has finished executing
	*/
	void GfxVkDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free)
	{
		if (commandBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo = VkInitializers::SubmitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		// Create fence to ensure that the command buffer has finished executing
		VkFenceCreateInfo fenceInfo = VkInitializers::FenceCreateInfo(VK_FLAGS_NONE);
		VkFence fence;
		VK_CHECK_RESULT(vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &fence));
		// Submit to the queue
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
		// Wait for the fence to signal that command buffer has finished executing
		VK_CHECK_RESULT(vkWaitForFences(m_logicalDevice, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
		vkDestroyFence(m_logicalDevice, fence, nullptr);
		if (free)
		{
			vkFreeCommandBuffers(m_logicalDevice, pool, 1, &commandBuffer);
		}
	}

	/**
	*
	* Wrapper to flush command buffer with default command buffer pool
	*/
	void GfxVkDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
	{
		return FlushCommandBuffer(commandBuffer, queue, m_commandPool, free);
	}

	/**
	* Check if an extension is supported by the (physical device)
	*
	* @param extension Name of the extension to check
	*
	* @return True if the extension is supported (present in the list read at device creation time)
	*/
	bool GfxVkDevice::ExtensionSupported(std::string extension)
	{
		return (std::find(m_supportedExtensions.begin(), m_supportedExtensions.end(), extension) != m_supportedExtensions.end());
	}

	/**
	* Select the best-fit depth format for this device from a list of possible depth (and stencil) formats
	*
	* @param checkSamplingSupport Check if the format can be sampled from (e.g. for shader reads)
	*
	* @return The depth format that best fits for the current device
	*
	* @throw Throws an exception if no depth format fits the requirements
	*/
	VkFormat GfxVkDevice::GetSupportedDepthFormat(bool checkSamplingSupport)
	{
		// All depth formats may be optional, so we need to find a suitable depth format to use
		std::vector<VkFormat> depthFormats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };
		for (auto& format : depthFormats)
		{
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &formatProperties);
			// Format must support depth stencil attachment for optimal tiling
			if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				if (checkSamplingSupport) {
					if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
						continue;
					}
				}
				return format;
			}
		}
		throw std::runtime_error("Could not find a matching depth format");
	}


}
}