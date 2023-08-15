#include "nbpch.h"
#include "Vulkan_RenderAPI.h"

#include "Nebula/Core/Application.h"
#include "Nebula/Scene/SceneRenderer.h"

#include "VulkanAPI.h"
#include "Vulkan_Framebuffer.h"
#include "Vulkan_Shader.h"

namespace Nebula {
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{

		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: NB_TRACE(pCallbackData->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: NB_INFO(pCallbackData->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: NB_WARN(pCallbackData->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:	NB_ERROR(pCallbackData->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT: NB_CRITICAL(pCallbackData->pMessage); break;
		}

		return VK_FALSE;
	}

	namespace Utils
	{
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
	}

	void Vulkan_RendererAPI::Init() {
		NB_PROFILE_FUNCTION();

		VulkanAPI::Init(debugCallback);

#if NB_DEBUG
		CreateDebugMessenger();
#endif
	}

	void Vulkan_RendererAPI::CreateDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = VulkanAPI::PopulateDebugMessenger(debugCallback);
		VkResult result = Utils::CreateDebugUtilsMessengerEXT(VulkanAPI::GetInstance(), &createInfo, nullptr, &m_DebugMessenger);
		NB_ASSERT(result == VK_SUCCESS, "Failed to set up debug messenger!");
	}

	void Vulkan_RendererAPI::Shutdown()
	{
		Utils::DestroyDebugUtilsMessengerEXT(VulkanAPI::GetInstance(), m_DebugMessenger, nullptr);
		VulkanAPI::Shutdown();
	}

	void Vulkan_RendererAPI::SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) 
	{
	}

	void Vulkan_RendererAPI::Clear() 
	{
		if (Vulkan_FrameBuffer* framebuffer = Vulkan_FrameBuffer::s_BindedInstance)
		{
			framebuffer->ClearAttachment(0, m_ClearColour);
			framebuffer->ClearDepthAttachment(0);
			return;
		}

		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		auto& image = context->m_Images[context->m_ImageIndex];
		
		VkCommandBuffer commandBuffer = VulkanAPI::BeginSingleUseCommand();

		VulkanAPI::TransitionImageLayout(image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		commandBuffer = VulkanAPI::BeginSingleUseCommand();
		VkClearColorValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };

		VkImageSubresourceRange subResourceRange = {};
		subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResourceRange.baseMipLevel = 0;
		subResourceRange.levelCount = 1;
		subResourceRange.baseArrayLayer = 0;
		subResourceRange.layerCount = 1;

		vkCmdClearColorImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &subResourceRange);
		VulkanAPI::EndSingleUseCommand(commandBuffer);

		VulkanAPI::TransitionImageLayout(image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	void Vulkan_RendererAPI::SetClearColour(float r, float g, float b, float a) 
	{
		m_ClearColour = { r, g, b, a };
	}

	void Vulkan_RendererAPI::SetClearColour(const glm::vec4& colour) 
	{
		m_ClearColour = colour;
	}

	void Vulkan_RendererAPI::SetBackfaceCulling(bool cull) 
	{
		m_BackFaceCulling = cull;
	}

	void Vulkan_RendererAPI::BeginRecording()
	{
		VulkanAPI::BeginCommandRecording();
		Vulkan_FrameBuffer::BeginRenderPass();
		
		vkCmdSetLineWidth(VulkanAPI::GetCommandBuffer(), m_LineWidth);
		vkCmdSetCullMode(VulkanAPI::GetCommandBuffer(), m_BackFaceCulling ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE);
	}

	void Vulkan_RendererAPI::EndRecording()
	{
		vkCmdEndRenderPass(VulkanAPI::GetCommandBuffer());
		VulkanAPI::EndCommandRecording();
	}
	
	void Vulkan_RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) 
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		vkCmdDrawIndexed(VulkanAPI::GetCommandBuffer(), count, 1, 0, 0, 0);
	}

	void Vulkan_RendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) 
	{
		vertexArray->Bind();
		uint32_t count = vertexCount ? vertexCount : vertexArray->GetIndexBuffer()->GetCount();
		vkCmdDraw(VulkanAPI::GetCommandBuffer(), count, 1, 0, 0);
	}

	void Vulkan_RendererAPI::SetLineWidth(float width) 
	{
		m_LineWidth = width;
	}
}