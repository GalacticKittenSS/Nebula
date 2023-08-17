#pragma once

#include <vulkan/vulkan.h>

namespace Nebula
{
	class VulkanImage;

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
		static void TransitionImageLayout(Ref<VulkanImage> image, VkImageLayout newLayout, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
		static void AllocateDescriptorSet(VkDescriptorSet& descriptorSet, const VkDescriptorSetLayout& layout);

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

		static VkDescriptorPool s_DescriptorPool;

		static bool s_FirstSubmit;
		static uint8_t s_FrameIndex;
		static bool s_CommandBufferRecording;

		friend class ImGuiLayer;
	};

	class VulkanBuffer
	{
	public:
		VulkanBuffer() = default;
		VulkanBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		~VulkanBuffer();
		
		void SetData(const void* data, uint32_t size, uint32_t offset = 0);
		VkBuffer& GetBuffer() { return m_Buffer; }
		VkDeviceSize GetSize() { return m_AlignedSize; }
		void* GetMemory() { return m_MappedMemory; }
	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_BufferMemory = VK_NULL_HANDLE;
		void* m_MappedMemory = nullptr;

		size_t m_AlignedSize = 0;
	};

	class VulkanImage
	{
	public:
		VulkanImage() = default;
		VulkanImage(VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect, int samples, uint32_t width, uint32_t height);
		~VulkanImage();

		static std::vector<Ref<VulkanImage>> CreateImageArray(VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect, int samples, uint32_t width, uint32_t height);
		static std::vector<Ref<VulkanImage>> CreateImageArray(std::vector<VkImage> images, std::vector<VkImageView> imageViews);

		const VkImage& GetImage() const { return m_Image; }
		const VkImageView& GetImageView() const { return m_ImageView; }
		const VkFormat& GetFormat() const { return m_ImageFormat; }
		const VkImageAspectFlags& GetAspectFlags() const { return m_AspectFlags; }
		VkImageLayout& GetLayout() { return m_ImageLayout; }

		static VkSampleCountFlagBits GetSampleFlags(int samples);
		static void CreateTextureImage(VkImageView& view, VkImage& image, VkDeviceMemory& memory, int samples, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect, uint32_t width, uint32_t height);
	private:
		VkImage m_Image = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;

		VkFormat m_ImageFormat;
		VkImageAspectFlags m_AspectFlags;
		VkImageLayout m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		bool m_OwnsImages = true;
	};

	using VulkanImageArray = std::vector<Ref<VulkanImage>>;
}