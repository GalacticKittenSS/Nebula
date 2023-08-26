#pragma once

#include <vulkan/vulkan.h>

extern "C"
{
	typedef struct VmaAllocator_T* VmaAllocator;
	typedef struct VmaAllocation_T* VmaAllocation;
}

namespace Nebula
{
	class Vulkan_Image;

	class VulkanAPI
	{
	public:
		static void Init(PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);
		static void Shutdown();

		static VkDebugUtilsMessengerCreateInfoEXT PopulateDebugMessenger(PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);

		static VkCommandBuffer BeginSingleUseCommand();
		static void EndSingleUseCommand(VkCommandBuffer commandBuffer);
		
		static void ResetFrame();
		static void BeginCommandRecording();
		static void EndCommandRecording();

		static uint32_t FindMemoryType(uint32_t filter, VkMemoryPropertyFlags properties);
		static void TransitionImageLayout(VkImage image, VkImageAspectFlags imageAspect, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
		static void TransitionImageLayout(Ref<Vulkan_Image> image, VkImageLayout newLayout, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
		static void AllocateDescriptorSet(VkDescriptorSet& descriptorSet, const VkDescriptorSetLayout& layout);

		static void SubmitResource(const std::function<void()>& func);

		static inline const VkInstance& GetInstance() { return s_Instance; }
		static inline const VkDevice& GetDevice() { return s_Device; }
		static inline const VkPhysicalDevice& GetPhysicalDevice() { return s_PhysicalDevice; }
		
		static inline const VkQueue& GetQueue() { return s_Queue; }
		static inline uint32_t GetQueueFamily() { return s_QueueFamily; }

		static inline const VkCommandBuffer& GetCommandBuffer() { return s_CommandBuffers[s_FrameIndex]; }
		static inline const VkSemaphore& GetRenderSemaphore() { return s_RenderSemaphores[s_FrameIndex]; }
		static inline const VkSemaphore& GetImageSemaphore() { return s_ImageSemaphores[s_FrameIndex]; }
		static inline const VkFence& GetFence() { return s_Fences[s_FrameIndex]; }

		static inline bool IsRecording() { return s_CommandBufferRecording; }
		static VkDescriptorPool s_DescriptorPool;
		static VmaAllocator s_Allocator;
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

		static std::vector<std::vector<std::function<void()>>> s_FreeResourceFuncs;

		static bool s_FirstSubmit;
		static uint8_t s_FrameIndex;
		static bool s_CommandBufferRecording;

		friend class ImGuiLayer;
	};

	class VulkanBuffer
	{
	public:
		VulkanBuffer() = default;
		VulkanBuffer(uint32_t size, VkBufferUsageFlags usage, bool hostAccess = true);
		~VulkanBuffer();
		
		void SetData(const void* data, uint32_t size, uint32_t offset = 0);
		VkBuffer& GetBuffer() { return m_Buffer; }
		VkDeviceSize GetSize() { return m_AlignedSize; }
		void* GetMemory() { return m_MappedMemory; }
	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		void* m_MappedMemory = nullptr;

		VmaAllocation m_Allocation;
		size_t m_AlignedSize = 0;
	};
}