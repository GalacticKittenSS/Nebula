#include "nbpch.h"
#include "Vulkan_CommandBuffer.h"

#include "VulkanAPI.h"

namespace Nebula
{
	Vulkan_CommandBuffer* Vulkan_CommandBuffer::s_Instance = nullptr;

	Vulkan_CommandBuffer::Vulkan_CommandBuffer()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VulkanAPI::GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkResult result = vkAllocateCommandBuffers(VulkanAPI::GetDevice(), &allocInfo, &m_CommandBuffer);
		NB_ASSERT(result == VK_SUCCESS, "Failed to allocate command buffers!");

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		
		result = vkCreateFence(VulkanAPI::GetDevice(), &fenceInfo, nullptr, &m_Fence);
		NB_ASSERT(result == VK_SUCCESS, "Failed to allocate command buffers!");
	}

	Vulkan_CommandBuffer::~Vulkan_CommandBuffer()
	{
		VulkanAPI::SubmitResource([fence = m_Fence] {
			vkDestroyFence(VulkanAPI::GetDevice(), fence, nullptr);
		});
	}

	void Vulkan_CommandBuffer::BeginRecording()
	{
		vkWaitForFences(VulkanAPI::GetDevice(), 1, &m_Fence, VK_TRUE, UINT64_MAX);

		vkResetFences(VulkanAPI::GetDevice(), 1, &m_Fence);
		vkResetCommandBuffer(m_CommandBuffer, 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkResult result = vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);
		NB_ASSERT(result == VK_SUCCESS, "Failed to begin recording command buffer!");

		m_Recording = true;
		s_Instance = this;
	}
		
	void Vulkan_CommandBuffer::EndRecording()
	{
		s_Instance = nullptr;
		m_Recording = false;

		VkResult result = vkEndCommandBuffer(m_CommandBuffer);
		NB_ASSERT(result == VK_SUCCESS, "Failed to record command buffer!");
	}
	
	void Vulkan_CommandBuffer::Submit()
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { VulkanAPI::GetSemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;

		VkSemaphore signalSemaphores[] = { VulkanAPI::GetRenderSemaphore() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VkResult result = vkQueueSubmit(VulkanAPI::GetQueue(), 1, &submitInfo, m_Fence);
		NB_ASSERT(result == VK_SUCCESS, "Failed to submit draw command buffer!");
	}
}