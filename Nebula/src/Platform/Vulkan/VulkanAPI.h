#pragma once

#include <vulkan/vulkan.h>

namespace Nebula
{
	class VulkanAPI
	{
	public:
		static void Init(PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);
		static void Shutdown();

		static VkDebugUtilsMessengerCreateInfoEXT PopulateDebugMessenger(PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);

		static VkCommandBuffer BeginSingleUseCommand();
		static void EndSingleUseCommand(VkCommandBuffer commandBuffer);
	
		static uint32_t FindMemoryType(uint32_t filter, VkMemoryPropertyFlags properties);
		static void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

		static const VkInstance& GetInstance() { return s_Instance; }
		static const VkDevice& GetDevice() { return s_Device; }
		static const VkPhysicalDevice& GetPhysicalDevice() { return s_PhysicalDevice; }
		
		static const VkQueue& GetQueue() { return s_Queue; }
		static uint32_t GetQueueFamily() { return s_QueueFamily; }

		static const VkCommandBuffer& GetCommandBuffer() { return s_CommandBuffers[m_FrameIndex]; }

		static const VkSemaphore& GetRenderSemaphore() { return s_RenderSemaphores[m_FrameIndex]; }
		static const VkSemaphore& GetImageSemaphore() { return s_ImageSemaphores[m_FrameIndex]; }
		static const VkFence& GetFence() { return s_Fences[m_FrameIndex]; }
	private:
		static void CreateLogicalDevice();
		static void CreateCommandBuffers();
		static void CreateSyncObjects();

		static VkPhysicalDevice PickPhysicalDevice();
	private:
		static VkInstance s_Instance;
		static VkDevice s_Device;
		static VkPhysicalDevice s_PhysicalDevice;

		static uint32_t s_QueueFamily;
		static VkQueue s_Queue;

		static VkCommandPool s_CommandPool;
		static std::vector<VkCommandBuffer> s_CommandBuffers;

		static std::vector<VkSemaphore> s_ImageSemaphores;
		static std::vector<VkSemaphore> s_RenderSemaphores;
		static std::vector<VkFence> s_Fences;

		static uint8_t m_FrameIndex;

		friend class Vulkan_RendererAPI;
		friend class ImGuiLayer;
	};
}