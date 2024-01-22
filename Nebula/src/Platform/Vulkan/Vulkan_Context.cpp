#include "nbpch.h"
#include "Vulkan_Context.h"

#include "Nebula/Renderer/Render_Command.h"
#include "Nebula/Scene/SceneRenderer.h"

#include "VulkanAPI.h"
#include "Vulkan_FrameBuffer.h"

#include <GLFW/glfw3.h>

namespace Nebula {
	namespace Utils
	{
		static void CopyImage(VkImage srcimage, VkImage dstimage, uint32_t width, uint32_t height)
		{
			VkCommandBuffer commandBuffer = VulkanAPI::BeginSingleUseCommand();

			VkImageCopy region{};

			region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.dstSubresource.mipLevel = 0;
			region.dstSubresource.baseArrayLayer = 0;
			region.dstSubresource.layerCount = 1;

			region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.srcSubresource.mipLevel = 0;
			region.srcSubresource.baseArrayLayer = 0;
			region.srcSubresource.layerCount = 1;

			region.dstOffset = { 0, 0, 0 };
			region.srcOffset = { 0, 0, 0 };
			region.extent = { width, height, 1 };

			vkCmdCopyImage(commandBuffer, srcimage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstimage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
			VulkanAPI::EndSingleUseCommand(commandBuffer);
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
	}

	Vulkan_Context::Vulkan_Context(GLFWwindow* windowHandle) 
		: m_WindowHandle(windowHandle)
	{
		NB_ASSERT(windowHandle, "Window Handle is NULL!");
	}

	void Vulkan_Context::Init() {
		NB_PROFILE_FUNCTION();
		
		// TODO: Move GLFW API to Window
		NB_ASSERT(glfwVulkanSupported(), "GLFW: Vulkan not supported!");

		VkResult result = glfwCreateWindowSurface(VulkanAPI::GetInstance(), m_WindowHandle, nullptr, &m_Surface);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create window surface");

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(VulkanAPI::GetPhysicalDevice(), &properties);

		uint8_t versionMajor = VK_API_VERSION_MAJOR(properties.apiVersion);
		uint8_t versionMinor = VK_API_VERSION_MINOR(properties.apiVersion);
		uint8_t versionPatch = VK_API_VERSION_PATCH(properties.apiVersion);

		NB_INFO("Vulkan Info:");
		NB_INFO("  Vendor: {0}", Utils::VendorIDToString(properties.vendorID));
		NB_INFO("  Renderer: {0}", properties.deviceName);
		NB_INFO("  Version: {0}.{1}.{2}", versionMajor, versionMinor, versionPatch);

		NB_ASSERT(versionMajor == 1 && versionMinor >= 2, "Nebula Requires Vulkan version 1.2 and Up!");

		CreateSwapChain();
		CreateImageViews();
		AcquireNextImage();
	}

	void Vulkan_Context::Shutdown()
	{
		NB_PROFILE_FUNCTION();

		VulkanAPI::SubmitResource([imageViews = m_ImageViews, swapchain = m_SwapChain, surface = m_Surface]()
		{
			for (auto imageView : imageViews)
				vkDestroyImageView(VulkanAPI::GetDevice(), imageView, nullptr);

			vkDestroySwapchainKHR(VulkanAPI::GetDevice(), swapchain, nullptr);
			vkDestroySurfaceKHR(VulkanAPI::GetInstance(), surface, nullptr);
		});
	}

	bool Vulkan_Context::AcquireNextImage()
	{
		VkResult result = vkAcquireNextImageKHR(VulkanAPI::GetDevice(), m_SwapChain, UINT64_MAX,
			VulkanAPI::GetImageSemaphore(), VK_NULL_HANDLE, &m_ImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			m_RecreateSwapChain = true;
			return false;
		}

		NB_ASSERT(result == VK_SUCCESS, "Failed to acquire swap chain image!");
		return true;
	}

	void Vulkan_Context::PresentCurrentImage()
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &VulkanAPI::GetRenderSemaphore();
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_SwapChain;
		presentInfo.pImageIndices = &m_ImageIndex;

		VkResult result = vkQueuePresentKHR(VulkanAPI::GetQueue(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			m_RecreateSwapChain = true;
			return;
		}

		NB_ASSERT(result == VK_SUCCESS, "Failed to present swap chain image!");
	}

	void Vulkan_Context::SwapBuffers() 
	{
		m_RecreateSwapChain = false;

		PresentCurrentImage();

		VulkanAPI::ResetFrame();
		AcquireNextImage();

		if (m_RecreateSwapChain)
		{
			RecreateSwapChain();
			AcquireNextImage();
			VulkanAPI::TransitionImageLayout(m_Images[m_ImageIndex], VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		}
	}

	void Vulkan_Context::SetVsync(bool vsync)
	{
		m_PresentMode = vsync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
		RecreateSwapChain();
		AcquireNextImage();
		m_RecreateSwapChain = true;
	}

	Vulkan_Context::SwapChainSupportDetails Vulkan_Context::QuerySwapChainSupport(VkPhysicalDevice device) 
	{
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

	VkSurfaceFormatKHR Vulkan_Context::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}

		return availableFormats[0];
	}

	VkPresentModeKHR Vulkan_Context::ChooseSwapPresentMode(VkPresentModeKHR targetPresentMode, const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == targetPresentMode)
				return targetPresentMode;
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
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(m_PresentMode, swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

		uint32_t minImageCount = swapChainSupport.capabilities.minImageCount + 1;

		if (swapChainSupport.capabilities.maxImageCount > 0 && m_SwapChainImageCount > swapChainSupport.capabilities.maxImageCount)
			minImageCount = swapChainSupport.capabilities.maxImageCount;
		
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(VulkanAPI::GetPhysicalDevice(), VulkanAPI::GetQueueFamily(), m_Surface, &presentSupport);
		NB_ASSERT(presentSupport == VK_TRUE);

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;
		createInfo.minImageCount = minImageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageArrayLayers = 1;
		createInfo.imageExtent = extent;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Assume that graphics family == present family
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = m_SwapChain;

		VkDevice device = VulkanAPI::GetDevice();
		VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_SwapChain);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create swap chain!");
		
		vkGetSwapchainImagesKHR(device, m_SwapChain, &m_SwapChainImageCount, nullptr);
		m_Images.resize(m_SwapChainImageCount);
		vkGetSwapchainImagesKHR(device, m_SwapChain, &m_SwapChainImageCount, m_Images.data());

		m_ImageFormat = surfaceFormat.format;
		m_Extent = extent;
	}

	void Vulkan_Context::CreateImageViews()
	{
		m_ImageViews.resize(m_SwapChainImageCount);

		for (size_t i = 0; i < m_SwapChainImageCount; i++)
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

	void Vulkan_Context::RecreateSwapChain() 
	{
		vkDeviceWaitIdle(VulkanAPI::GetDevice());
		CleanUpSwapChain();

		CreateSwapChain();
		CreateImageViews();

		m_ImageArray = Vulkan_Image::CreateImageArray(m_Images, m_ImageViews, m_Extent.width, m_Extent.height, m_ImageFormat);
	}

	void Vulkan_Context::CleanUpSwapChain()
	{
		VulkanAPI::SubmitResource([imageViews = m_ImageViews, swapchain = m_SwapChain]()
		{
			vkDeviceWaitIdle(VulkanAPI::GetDevice());

			for (auto imageView : imageViews)
				vkDestroyImageView(VulkanAPI::GetDevice(), imageView, nullptr);
			
			vkDestroySwapchainKHR(VulkanAPI::GetDevice(), swapchain, nullptr);
		});	
	}
}