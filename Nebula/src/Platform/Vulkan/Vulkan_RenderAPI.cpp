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
		NB_TRACE("Validation Layer: {}", pCallbackData->pMessage);
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

	void Vulkan_RendererAPI::SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
	}

	void Vulkan_RendererAPI::Clear() 
	{
		if (Vulkan_FrameBuffer* framebuffer = Vulkan_FrameBuffer::s_BindedInstance)
		{
			framebuffer->ClearAttachment(0, 0);
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
	}

	void Vulkan_RendererAPI::SetClearColour(const glm::vec4& colour) 
	{
	}

	void Vulkan_RendererAPI::SetBackfaceCulling(bool cull) 
	{
		VkCommandBuffer commandBuffer = VulkanAPI::BeginSingleUseCommand();
		vkCmdSetCullMode(commandBuffer, cull ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE);
		VulkanAPI::EndSingleUseCommand(commandBuffer);
	}

	void Vulkan_RendererAPI::recordCommandBuffer(Ref<VertexArray> array, VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			NB_CRITICAL("Failed to begin recording command buffer!");

		Vulkan_FrameBuffer* framebuffer = Vulkan_FrameBuffer::s_BindedInstance;

		VkExtent2D extent;
		extent.width = framebuffer->m_Specifications.Width;
		extent.height = framebuffer->m_Specifications.Height;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = framebuffer->m_RenderPass;
		renderPassInfo.framebuffer = framebuffer->m_Framebuffer[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;
		
		uint32_t attachmentCount = framebuffer->m_ColourAttachments.size();
		if (framebuffer->m_DepthAttachment)
			attachmentCount++;

		std::vector<VkClearValue> clearValues(attachmentCount);
		renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		Vulkan_Shader::BindPipeline();

		array->Bind();

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdDrawIndexed(commandBuffer, array->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffer);

		VkResult result = vkEndCommandBuffer(commandBuffer);
		NB_ASSERT(result == VK_SUCCESS, "Failed to record command buffer!");
	}

	void Vulkan_RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) 
	{
		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		
		uint32_t imageIndex = context->m_ImageIndex;
		if (imageIndex == (uint32_t)-1)
			return;
		
		vkWaitForFences(VulkanAPI::GetDevice(), 1, &VulkanAPI::GetFence(), VK_TRUE, UINT64_MAX);
		vkResetFences(VulkanAPI::GetDevice(), 1, &VulkanAPI::GetFence());
		
		VkCommandBuffer commandBuffer = VulkanAPI::GetCommandBuffer();
		vkResetCommandBuffer(commandBuffer, 0);
		recordCommandBuffer(vertexArray, commandBuffer, imageIndex);
		
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { VulkanAPI::GetImageSemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VkSemaphore signalSemaphores[] = { VulkanAPI::GetRenderSemaphore() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VkResult result = vkQueueSubmit(VulkanAPI::GetQueue(), 1, &submitInfo, VulkanAPI::GetFence());
		NB_ASSERT(result == VK_SUCCESS, "Failed to submit draw command buffer!");
	}

	void Vulkan_RendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) 
	{
	}

	void Vulkan_RendererAPI::SetLineWidth(float width) 
	{
		VkCommandBuffer commandBuffer = VulkanAPI::BeginSingleUseCommand();
		vkCmdSetLineWidth(commandBuffer, width);
		VulkanAPI::EndSingleUseCommand(commandBuffer);
	}
}