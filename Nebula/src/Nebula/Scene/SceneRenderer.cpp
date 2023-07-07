#include "nbpch.h"
#include "SceneRenderer.h"

#include "Nebula/Core/Application.h"
#include "Nebula/Renderer/Render_Command.h"
#include "Nebula/Renderer/Shader.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include <map>
#include <optional>
#include <set>

namespace Nebula
{
	struct VulkanData
	{
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkImageView> swapChainImageViews;
		VkRenderPass renderPass;
		
		Ref<Shader> shader;
		
		std::vector<VkFramebuffer> swapChainFramebuffers;
		
		uint32_t currentFrame = 0;
		bool framebufferResized = false;
	};
	static VulkanData s_VKData;

	SceneRenderer::Settings SceneRenderer::m_Settings = {};

	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		s_VKData.framebufferResized = true;
	}

	void SceneRenderer::Setup()
	{
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();

		s_VKData.shader = Shader::Create("Resources/shaders/Vulkan.glsl");
		
		CreateFramebuffers();
		CreateSyncObjects();
	}

	void** SceneRenderer::GetGraphicsQueue()
	{
		return (void**)&s_VKData.graphicsQueue;
	}

	void** SceneRenderer::GetPresentQueue()
	{
		return (void**)&s_VKData.presentQueue;
	}

	void* SceneRenderer::GetRenderPass()
	{
		return s_VKData.renderPass;
	}

	void SceneRenderer::RecreateSwapChain() {
		int width = 0, height = 0;
		Window& window = Application::Get().GetWindow();
		glfwGetFramebufferSize((GLFWwindow*)window.GetNativeWindow(), &width, &height);
		
		vkDeviceWaitIdle((VkDevice)RenderCommand::GetDevice());
		CleanUpSwapChain();

		CreateSwapChain();
		CreateImageViews();
		CreateFramebuffers();
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			Window& window = Application::Get().GetWindow();
			glfwGetFramebufferSize((GLFWwindow*)window.GetNativeWindow(), &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) {
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

		VkBool32 presentSupport = false;
		Window& window = Application::Get().GetWindow();
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, (VkSurfaceKHR)window.GetSurface(), &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		return indices;
	}

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	}; 
	
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) {
		Window& window = Application::Get().GetWindow();

		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, (VkSurfaceKHR)window.GetSurface(), &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, (VkSurfaceKHR)window.GetSurface(), &formatCount, nullptr);

		if (formatCount) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, (VkSurfaceKHR)window.GetSurface(), &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, (VkSurfaceKHR)window.GetSurface(), &presentModeCount, nullptr);

		if (presentModeCount) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, (VkSurfaceKHR)window.GetSurface(), &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	void SceneRenderer::CreateSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport((VkPhysicalDevice)RenderCommand::GetPhysicalDevice());

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

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
		
		Window& window = Application::Get().GetWindow();
		createInfo.surface = (VkSurfaceKHR)window.GetSurface();

		QueueFamilyIndices indices = FindQueueFamilies((VkPhysicalDevice)RenderCommand::GetPhysicalDevice());
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VkResult result = vkCreateSwapchainKHR((VkDevice)RenderCommand::GetDevice(), &createInfo, nullptr, &s_VKData.swapChain);
		if (result != VK_SUCCESS) {
			NB_ERROR("Failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR((VkDevice)RenderCommand::GetDevice(), s_VKData.swapChain, &imageCount, nullptr);
		s_VKData.swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR((VkDevice)RenderCommand::GetDevice(), s_VKData.swapChain, &imageCount, s_VKData.swapChainImages.data());

		s_VKData.swapChainImageFormat = surfaceFormat.format;
		s_VKData.swapChainExtent = extent;
	}

	void SceneRenderer::CreateImageViews()
	{
		s_VKData.swapChainImageViews.resize(s_VKData.swapChainImages.size());

		for (size_t i = 0; i < s_VKData.swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = s_VKData.swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = s_VKData.swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView((VkDevice)RenderCommand::GetDevice(), &createInfo, nullptr, &s_VKData.swapChainImageViews[i]) != VK_SUCCESS) {
				NB_CRITICAL("Failed to create image views!");
			}
		}
	}

	VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size() * sizeof(uint32_t);
		createInfo.pCode = code.data();

		VkShaderModule shaderModule;
		VkResult result = vkCreateShaderModule((VkDevice)RenderCommand::GetDevice(), &createInfo, nullptr, &shaderModule);
		if (result != VK_SUCCESS)
		{
			NB_ERROR("Failed to create shader module!");
		}

		return shaderModule;
	}

	void SceneRenderer::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = s_VKData.swapChainImageFormat;
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

		VkResult result = vkCreateRenderPass((VkDevice)RenderCommand::GetDevice(), &renderPassInfo, nullptr, &s_VKData.renderPass);
		if (result != VK_SUCCESS) {
			NB_CRITICAL("Failed to create render pass!");
		}
	}

	void SceneRenderer::CreateFramebuffers()
	{
		s_VKData.swapChainFramebuffers.resize(s_VKData.swapChainImageViews.size());

		for (size_t i = 0; i < s_VKData.swapChainImageViews.size(); i++)
		{
			VkImageView attachments[] = {
			   s_VKData.swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = s_VKData.renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = s_VKData.swapChainExtent.width;
			framebufferInfo.height = s_VKData.swapChainExtent.height;
			framebufferInfo.layers = 1;

			VkResult result = vkCreateFramebuffer((VkDevice)RenderCommand::GetDevice(), &framebufferInfo, nullptr, &s_VKData.swapChainFramebuffers[i]);
			if (result != VK_SUCCESS)
			{
				NB_CRITICAL("Failed to create framebuffer");
			}
		}
	}

	void SceneRenderer::Render()
	{
		RenderCommand::DrawIndexed(nullptr);
	}

	void SceneRenderer::CleanUpSwapChain() 
	{
		for (auto framebuffer : s_VKData.swapChainFramebuffers) {
			vkDestroyFramebuffer((VkDevice)RenderCommand::GetDevice(), framebuffer, nullptr);
		}

		for (auto imageView : s_VKData.swapChainImageViews) {
			vkDestroyImageView((VkDevice)RenderCommand::GetDevice(), imageView, nullptr);
		}

		vkDestroySwapchainKHR((VkDevice)RenderCommand::GetDevice(), s_VKData.swapChain, nullptr);
	}

	void SceneRenderer::CleanUp()
	{
		vkDeviceWaitIdle((VkDevice)RenderCommand::GetDevice());
		CleanUpSwapChain();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore((VkDevice)RenderCommand::GetDevice(), s_VKData.imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore((VkDevice)RenderCommand::GetDevice(), s_VKData.renderFinishedSemaphores[i], nullptr);
			vkDestroyFence((VkDevice)RenderCommand::GetDevice(), s_VKData.inFlightFences[i], nullptr);
		}

		vkDestroyRenderPass((VkDevice)RenderCommand::GetDevice(), s_VKData.renderPass, nullptr);
	}
}