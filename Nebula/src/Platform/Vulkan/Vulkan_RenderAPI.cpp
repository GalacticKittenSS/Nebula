#include "nbpch.h"
#include "Vulkan_RenderAPI.h"

#include "Nebula/Core/Application.h"
#include "Nebula/Scene/SceneRenderer.h"

#include <map>
#include <set>

namespace Nebula {
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		NB_TRACE("Validation Layer: {}", pCallbackData->pMessage);
		return VK_FALSE;
	}

	static bool CheckValidationLayerSupport(std::vector<const char*> validationLayers)
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

			if (!layerFound) {
				return false;
			}
		}

		return true;

	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	static VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	void Vulkan_RendererAPI::Init() {
		NB_PROFILE_FUNCTION();

		if (!CheckValidationLayerSupport(m_ValidationLayers))
		{
			NB_ERROR("validation layers requested, but not available!");
			return;
		}

		ApplicationSpecification spec = Application::Get().GetSpecification();

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

		auto extensions = GetExtensions();
		createInfo.enabledExtensionCount = (uint32_t)extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
#ifdef NB_DEBUG
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
		createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#else
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
#endif // NB_DEBUG

		NB_INFO("Initializing Vulkan");
		VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
		NB_ASSERT(result == VK_SUCCESS, "Could Not Initialise Vulkan");

		CreateDebugMessenger();
		CreateLogicalDevice();
		CreateCommandBuffers();
	}

	std::vector<const char*> Vulkan_RendererAPI::GetExtensions()
	{
		uint32_t windowExtensionCount = 0;
		const char** windowExtensions = Window::GetExtensions(windowExtensionCount);
		std::vector<const char*> extensions(windowExtensions, windowExtensions + windowExtensionCount);

#ifdef NB_DEBUG
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		return extensions;
	}

	void Vulkan_RendererAPI::CreateDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		VkResult result = CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
		NB_ASSERT(result == VK_SUCCESS, "Failed to set up debug messenger!");
	}

	Vulkan_RendererAPI::QueueFamilyIndices Vulkan_RendererAPI::FindQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		uint32_t i = 0;
		for (; i < queueFamilies.size(); i++)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
				break;
			}
		}

		VkBool32 presentSupport = true;
		//Window& window = Application::Get().GetWindow();
		//vkGetPhysicalDeviceSurfaceSupportKHR(device, i, (VkSurfaceKHR)window.GetSurface(), &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		return indices;
	}

	bool Vulkan_RendererAPI::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());
		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	bool Vulkan_RendererAPI::IsDeviceSuitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = FindQueueFamilies(device);
		bool extensionsSupported = CheckDeviceExtensionSupport(device);

		return indices.isComplete() && extensionsSupported;
	}

	uint16_t Vulkan_RendererAPI::RateDeviceSuitability(VkPhysicalDevice device)
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

	VkPhysicalDevice Vulkan_RendererAPI::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

		NB_ASSERT(deviceCount, "Failed to find GPUs with Vulkan support!");
		
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		std::map<uint16_t, VkPhysicalDevice> candidates;

		for (const auto& device : devices)
		{
			uint8_t score = RateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}

		auto& bestCandidate = candidates.rbegin();
		if (bestCandidate->first > 0)
			return bestCandidate->second;
		
		NB_ASSERT(false, "Failed to find suitable GPU!");
		return VK_NULL_HANDLE;
	}

	void Vulkan_RendererAPI::CreateLogicalDevice()
	{
		m_PhysicalDevice = PickPhysicalDevice();
		m_QueueIndices = FindQueueFamilies(m_PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { m_QueueIndices.graphicsFamily.value(), m_QueueIndices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = (uint32_t)m_DeviceExtensions.size();
		createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

#ifdef NB_DEBUG
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
		createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
#endif // NB_DEBUG

		VkResult result = vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create logical device!");
		
		vkGetDeviceQueue(m_Device, m_QueueIndices.graphicsFamily.value(), 0, (VkQueue*)SceneRenderer::GetGraphicsQueue());
		vkGetDeviceQueue(m_Device, m_QueueIndices.presentFamily.value(), 0, (VkQueue*)SceneRenderer::GetPresentQueue());
	}

	void Vulkan_RendererAPI::CreateSyncObjects()
	{
		m_ImageSemaphores.resize(2);
		m_RenderSemaphores.resize(2);
		m_Fences.resize(2);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < 2; i++)
		{
			if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_RenderSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_Device, &fenceInfo, nullptr, &m_Fences[i]) != VK_SUCCESS) 
			{
				NB_ERROR("Failed to create semaphores!");
				NB_ASSERT(false);
			}
		}
	}

	void Vulkan_RendererAPI::Shutdown()
	{
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
		vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
		vkDestroyDevice(m_Device, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
	}

	void Vulkan_RendererAPI::CreateCommandBuffers()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_QueueIndices.graphicsFamily.value();

		VkResult result = vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create command pool!");

		m_CommandBuffers.resize(2);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 2;

		result = vkAllocateCommandBuffers(m_Device, &allocInfo, m_CommandBuffers.data());
		NB_ASSERT(result == VK_SUCCESS, "Failed to allocate command buffers!");
	}

	void Vulkan_RendererAPI::SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		VkViewport viewport{};
		viewport.x = x;
		viewport.y = y;
		viewport.width = (float)width;
		viewport.height = (float)height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_CommandBuffers[m_VKData.currentFrame], 0, 1, &viewport);

		VkExtent2D extent;
		extent.height = height;
		extent.width = width;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(m_CommandBuffers[m_VKData.currentFrame], 0, 1, &scissor);
	}

	void Vulkan_RendererAPI::Clear() 
	{
	}

	void Vulkan_RendererAPI::SetClearColour(float r, float g, float b, float a) {
	}

	void Vulkan_RendererAPI::SetClearColour(const glm::vec4& colour) {
	}

	void Vulkan_RendererAPI::SetBackfaceCulling(bool cull) {
	}

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			NB_CRITICAL("Failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = (VkRenderPass)SceneRenderer::GetRenderPass();
		renderPassInfo.framebuffer = s_VKData.swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = s_VKData.swapChainExtent;

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipeline)s_VKData.shader->GetPipeline());

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)s_VKData.swapChainExtent.width;
		viewport.height = (float)s_VKData.swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = s_VKData.swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffer);

		VkResult result = vkEndCommandBuffer(commandBuffer);
		NB_ASSERT(result == VK_SUCCESS, "Failed to record command buffer!");
	}

	void Vulkan_RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) {
		vkWaitForFences(m_Device, 1, &m_Fences[s_VKData.currentFrame], VK_TRUE, UINT64_MAX);

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_Device, s_VKData.swapChain, UINT64_MAX, m_ImageSemaphores[s_VKData.currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			SceneRenderer::RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			NB_CRITICAL("Failed to acquire swap chain image!");
		}

		vkResetFences(m_Device, 1, & m_Fences[s_VKData.currentFrame]);

		vkResetCommandBuffer(m_CommandBuffers[s_VKData.currentFrame], 0);
		recordCommandBuffer(m_CommandBuffers[s_VKData.currentFrame], imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { s_VKData.imageAvailableSemaphores[s_VKData.currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers[s_VKData.currentFrame];

		VkSemaphore signalSemaphores[] = { m_RenderSemaphores[s_VKData.currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		result = vkQueueSubmit(s_VKData.graphicsQueue, 1, &submitInfo, m_Fences[s_VKData.currentFrame]);
		NB_ASSERT(result == VK_SUCCESS, "Failed to submit draw command buffer!");
		
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { s_VKData.swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(s_VKData.presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || s_VKData.framebufferResized) {
			s_VKData.framebufferResized = false;
			SceneRenderer::RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS) {
			NB_CRITICAL("Failed to present swap chain image!");
		}

		s_VKData.currentFrame = (s_VKData.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void Vulkan_RendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
	}

	void Vulkan_RendererAPI::SetLineWidth(float width) {
	}
}