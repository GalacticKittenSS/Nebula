#include "nbpch.h"
#include "Vulkan_RenderAPI.h"

#include "Nebula/Core/Application.h"
#include "Nebula/Scene/SceneRenderer.h"

#include "VulkanAPI.h"
#include "Vulkan_Framebuffer.h"
#include "Vulkan_Shader.h"

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
		Vulkan_Context* context = (Vulkan_Context*)Application::Get().GetWindow().GetContext();
		
		



	}

	void Vulkan_RendererAPI::SetClearColour(float r, float g, float b, float a) 
	{
	}

	void Vulkan_RendererAPI::SetClearColour(const glm::vec4& colour) 
	{
	}

	void Vulkan_RendererAPI::SetBackfaceCulling(bool cull) 
	{
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
		
		VkClearValue clearValues[2];
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues;

		// shader = Vulkan_Shader::s_BindedInstance
		Ref<Vulkan_Shader> shader = std::static_pointer_cast<Vulkan_Shader>(SceneRenderer::GetShader());

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->m_GraphicsPipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->m_PipelineLayout, 0, 1, &shader->m_DescriptorSet, 0, nullptr);

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