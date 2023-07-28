#include "nbpch.h"
#include "Vulkan_Context.h"

#include "Nebula/Renderer/Render_Command.h"
#include "Nebula/Scene/SceneRenderer.h"

#include "VulkanAPI.h"

#include <GLFW/glfw3.h>

namespace Nebula {
	Vulkan_Context::Vulkan_Context(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle) {
		NB_ASSERT(windowHandle, "Window Handle is NULL!")
	}

	static std::string VendorIDToString(uint32_t vendor)
	{
		switch (vendor)
		{
		case 0x1002: return "AMD";
		case 0x1010: return "ImgTec";
		case 0x10DE: return "NVIDIA";
		case 0x13B5: return "ARM";
		case 0x5143: return "Qualcomm";
		case 0x8086: return "INTEL";
		}

		return "Unknown";
	}

	void Vulkan_Context::Init() {
		NB_PROFILE_FUNCTION();

		VkResult result = glfwCreateWindowSurface(VulkanAPI::GetInstance(), m_WindowHandle, nullptr, &m_Surface);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create window surface");

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(VulkanAPI::GetPhysicalDevice(), &properties);

		uint8_t versionMajor = VK_API_VERSION_MAJOR(properties.apiVersion);
		uint8_t versionMinor = VK_API_VERSION_MINOR(properties.apiVersion);
		uint8_t versionPatch = VK_API_VERSION_PATCH(properties.apiVersion);

		NB_INFO("Vulkan Info:");
		NB_INFO("  Vendor: {0}", VendorIDToString(properties.vendorID));
		NB_INFO("  Renderer: {0}", properties.deviceName);
		NB_INFO("  Version: {0}.{1}.{2}", versionMajor, versionMinor, versionPatch);

		NB_ASSERT(versionMajor == 1 && versionMinor >= 2, "Nebula Requires Vulkan version 1.2 and Up!");

		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
	}

	void Vulkan_Context::Shutdown()
	{
		NB_PROFILE_FUNCTION();

		CleanUpSwapChain();
		vkDestroySurfaceKHR(VulkanAPI::GetInstance(), m_Surface, nullptr);
	}

	void Vulkan_Context::SwapBuffers() 
	{
		VkResult result = vkAcquireNextImageKHR(VulkanAPI::GetDevice(), m_SwapChain, UINT64_MAX,
			VulkanAPI::GetImageSemaphore(), VK_NULL_HANDLE, &m_ImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return;
		}

		NB_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire swap chain image!");
		
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &VulkanAPI::GetRenderSemaphore();

		VkSwapchainKHR swapChains[] = { m_SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_ImageIndex;

		result = vkQueuePresentKHR(VulkanAPI::GetQueue(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			RecreateSwapChain();
			return;
		}
		
		NB_ASSERT(result == VK_SUCCESS, "Failed to present swap chain image!");
	}

	Vulkan_Context::SwapChainSupportDetails Vulkan_Context::QuerySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

		if (formatCount) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

		if (presentModeCount) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR Vulkan_Context::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR Vulkan_Context::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Vulkan_Context::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
		{
			return capabilities.currentExtent;
		}
		else 
		{
			int width, height;
			glfwGetFramebufferSize(m_WindowHandle, &width, &height);
			VkExtent2D actualExtent = { (uint32_t)width, (uint32_t)height };

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	void Vulkan_Context::CreateSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(VulkanAPI::GetPhysicalDevice());

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.surface = m_Surface;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Assume that graphics family == present family
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VkDevice device = VulkanAPI::GetDevice();
		VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_SwapChain);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create swap chain!");
		
		vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, nullptr);
		m_Images.resize(imageCount);
		vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, m_Images.data());

		m_ImageFormat = surfaceFormat.format;
		m_Extent = extent;
	}

	void Vulkan_Context::CreateImageViews()
	{
		m_ImageViews.resize(m_Images.size());

		for (size_t i = 0; i < m_Images.size(); i++) 
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_Images[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_ImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VkResult result = vkCreateImageView(VulkanAPI::GetDevice(), &createInfo, nullptr, &m_ImageViews[i]);
			NB_ASSERT(result == VK_SUCCESS, "Failed to create image views!");
		}
	}

	void Vulkan_Context::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_ImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(VulkanAPI::GetDevice(), &renderPassInfo, nullptr, &m_RenderPass);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create render pass!");
	}

	void Vulkan_Context::RecreateSwapChain() 
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_WindowHandle, &width, &height);

		vkDeviceWaitIdle(VulkanAPI::GetDevice());
		CleanUpSwapChain();

		CreateSwapChain();
		CreateImageViews();
	}

	void Vulkan_Context::CleanUpSwapChain()
	{
		VkDevice device = VulkanAPI::GetDevice();
		vkDeviceWaitIdle(device);

		for (auto imageView : m_ImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		vkDestroyRenderPass(device, m_RenderPass, nullptr);
		vkDestroySwapchainKHR(device, m_SwapChain, nullptr);
	}
}