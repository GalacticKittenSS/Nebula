#include "nbpch.h"
#include "VulkanAPI.h"

#include "Nebula/Core/Window.h"

#include <map>
#include <set>

namespace Nebula
{
	static const uint32_t g_MaxFrames = 3;

	namespace Utils
	{
		static std::vector<const char*> GetValidationLayers()
		{
			return { "VK_LAYER_KHRONOS_validation" };
		}

		static bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers)
		{
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			for (const char* layerName : validationLayers) {
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers) {
					if (strcmp(layerName, layerProperties.layerName) == 0) {
						layerFound = true;
						break;
					}
				}

				if (!layerFound)
					return false;
			}

			return true;
		}

		static std::vector<const char*> GetDeviceExtensions()
		{
			return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		}

		std::vector<const char*> GetExtensions()
		{
			uint32_t windowExtensionCount = 0;
			const char** windowExtensions = Window::GetExtensions(windowExtensionCount);
			std::vector<const char*> extensions(windowExtensions, windowExtensions + windowExtensionCount);

#ifdef NB_DEBUG
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
			return extensions;
		}

		uint32_t FindGraphicsFamily(VkPhysicalDevice device)
		{
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			for (uint32_t i = 0; i < queueFamilies.size(); i++)
			{
				if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
					return i;
			}

			NB_ASSERT(false);
			return -1;
		}

		bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			std::vector<const char*> deviceExtensions = GetDeviceExtensions();
			std::set<std::string> requiredExtensions = { deviceExtensions.begin(), deviceExtensions.end() };

			for (const auto& extension : availableExtensions)
				requiredExtensions.erase(extension.extensionName);

			return requiredExtensions.empty();
		}

		bool IsDeviceSuitable(VkPhysicalDevice device)
		{
			uint32_t indices = FindGraphicsFamily(device);
			bool extensionsSupported = CheckDeviceExtensionSupport(device);

			VkPhysicalDeviceFeatures supportedFeatures;
			vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

			return indices != (uint32_t)-1 && extensionsSupported && supportedFeatures.samplerAnisotropy;
		}

		uint16_t RateDeviceSuitability(VkPhysicalDevice device)
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);

			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			if (!deviceFeatures.geometryShader || !IsDeviceSuitable(device))
				return 0;

			uint16_t score = 0;
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += 1000;

			score += deviceProperties.limits.maxImageDimension2D;
			return score;
		}
	}

	VkInstance VulkanAPI::s_Instance = VK_NULL_HANDLE;
	VkDevice VulkanAPI::s_Device = VK_NULL_HANDLE; 
	VkPhysicalDevice VulkanAPI::s_PhysicalDevice = VK_NULL_HANDLE;
	
	uint32_t VulkanAPI::s_QueueFamily = NULL;
	VkQueue VulkanAPI::s_Queue = VK_NULL_HANDLE;
	
	VkCommandPool VulkanAPI::s_CommandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> VulkanAPI::s_CommandBuffers = {};

	std::vector<VkSemaphore> VulkanAPI::s_ImageSemaphores = {};
	std::vector<VkSemaphore> VulkanAPI::s_RenderSemaphores = {};
	std::vector<VkFence> VulkanAPI::s_Fences = {};

	uint8_t VulkanAPI::m_FrameIndex = 0;

	void VulkanAPI::Init(PFN_vkDebugUtilsMessengerCallbackEXT debugCallback)
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Nebula Storm";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Nebula";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = Utils::GetExtensions();
		createInfo.enabledExtensionCount = (uint32_t)extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef NB_DEBUG
		std::vector<const char*> validationLayers = Utils::GetValidationLayers();
		NB_ASSERT(Utils::CheckValidationLayerSupport(validationLayers), "Validation layers requested, but not available!");

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = PopulateDebugMessenger(debugCallback);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#else
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
#endif // NB_DEBUG

		NB_INFO("Initializing Vulkan");
		VkResult result = vkCreateInstance(&createInfo, nullptr, &s_Instance);
		NB_ASSERT(result == VK_SUCCESS, "Could Not Initialise Vulkan");
		
		CreateLogicalDevice();
		CreateCommandBuffers();
		CreateSyncObjects();
	}

	VkDebugUtilsMessengerCreateInfoEXT VulkanAPI::PopulateDebugMessenger(PFN_vkDebugUtilsMessengerCallbackEXT debugCallback)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		return createInfo;
	}

	VkPhysicalDevice VulkanAPI::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(s_Instance, &deviceCount, nullptr);

		NB_ASSERT(deviceCount, "Failed to find GPUs with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(s_Instance, &deviceCount, devices.data());

		std::map<uint16_t, VkPhysicalDevice> candidates;

		for (const auto& device : devices)
		{
			uint16_t score = Utils::RateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}

		auto& bestCandidate = candidates.rbegin();
		if (bestCandidate->first > 0)
			return bestCandidate->second;

		NB_ASSERT(false, "Failed to find suitable GPU!");
		return VK_NULL_HANDLE;
	}

	void VulkanAPI::CreateLogicalDevice()
	{
		s_PhysicalDevice = PickPhysicalDevice();
		s_QueueFamily = Utils::FindGraphicsFamily(s_PhysicalDevice);

		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = s_QueueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pQueueCreateInfos = &queueCreateInfo;

		std::vector<const char*> deviceExtensions = Utils::GetDeviceExtensions();
		createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		createInfo.pEnabledFeatures = &deviceFeatures;

#ifdef NB_DEBUG
		std::vector<const char*> validatioLayers = Utils::GetValidationLayers();
		createInfo.enabledLayerCount = static_cast<uint32_t>(validatioLayers.size());
		createInfo.ppEnabledLayerNames = validatioLayers.data();
#else
		createInfo.enabledLayerCount = 0;
#endif // NB_DEBUG

		VkResult result = vkCreateDevice(s_PhysicalDevice, &createInfo, nullptr, &s_Device);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create logical device!");

		vkGetDeviceQueue(s_Device, s_QueueFamily, 0, &s_Queue);
	}
	
	void VulkanAPI::CreateCommandBuffers()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = s_QueueFamily;

		VkResult result = vkCreateCommandPool(s_Device, &poolInfo, nullptr, &s_CommandPool);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create command pool!");

		s_CommandBuffers.resize(g_MaxFrames);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = s_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = g_MaxFrames;

		result = vkAllocateCommandBuffers(s_Device, &allocInfo, s_CommandBuffers.data());
		NB_ASSERT(result == VK_SUCCESS, "Failed to allocate command buffers!");
	}
	
	void VulkanAPI::CreateSyncObjects()
	{
		s_ImageSemaphores.resize(g_MaxFrames);
		s_RenderSemaphores.resize(g_MaxFrames);
		s_Fences.resize(g_MaxFrames);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.flags = VK_SEMAPHORE_TYPE_BINARY;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < g_MaxFrames; i++)
		{
			if (vkCreateSemaphore(s_Device, &semaphoreInfo, nullptr, &s_ImageSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(s_Device, &semaphoreInfo, nullptr, &s_RenderSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(s_Device, &fenceInfo, nullptr, &s_Fences[i]) != VK_SUCCESS)
			{
				NB_ERROR("Failed to create semaphores!");
				NB_ASSERT(false);
			}
		}
	}

	void VulkanAPI::Shutdown()
	{
		vkDeviceWaitIdle(s_Device);

		for (size_t i = 0; i < g_MaxFrames; i++)
		{
			vkDestroySemaphore(s_Device, s_ImageSemaphores[i], nullptr);
			vkDestroySemaphore(s_Device, s_RenderSemaphores[i], nullptr);
			vkDestroyFence(s_Device, s_Fences[i], nullptr);
		}

		vkDestroyCommandPool(s_Device, s_CommandPool, nullptr);
		vkDestroyDevice(s_Device, nullptr);
		vkDestroyInstance(s_Instance, nullptr);
	}

	VkCommandBuffer VulkanAPI::BeginSingleUseCommand()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = s_CommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(s_Device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}

	void VulkanAPI::EndSingleUseCommand(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(s_Queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(s_Queue);

		vkFreeCommandBuffers(s_Device, s_CommandPool, 1, &commandBuffer);
	}
	
	uint32_t VulkanAPI::FindMemoryType(uint32_t filter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(VulkanAPI::GetPhysicalDevice(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if (filter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}

		NB_ASSERT(false, "Failed to find suitable memory type!");
		return -1;
	}

	void VulkanAPI::TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer commandBuffer = BeginSingleUseCommand();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0; // TODO
		barrier.dstAccessMask = 0; // TODO

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = 0;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

		}
		else
			NB_ASSERT(false, "Unsupported layout transition!");

		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		VulkanAPI::EndSingleUseCommand(commandBuffer);
	}
}