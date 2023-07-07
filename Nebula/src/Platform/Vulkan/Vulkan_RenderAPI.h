#pragma once

#include "Nebula/renderer/Renderer_API.h"

#include <vulkan/vulkan.h>
#include <optional>

namespace Nebula {
	class Vulkan_RendererAPI : public RendererAPI {
	public:
		void Init() override;
		void Shutdown() override;
		void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void Clear() override;
		void SetClearColour(float r, float g, float b, float a) override;
		void SetClearColour(const glm::vec4& colour) override;

		void SetBackfaceCulling(bool) override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) override;
		void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;

		void SetLineWidth(float width) override;
		const void* GetInstance() const override { return m_Instance; }
		const void* GetDevice() const override { return m_Device; }
		const void* GetPhysicalDevice() const override { return m_PhysicalDevice; }
		const void* GetCommandBuffer(uint32_t frame = 0) const override { return m_CommandBuffers[frame]; }
	private:
		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete() {
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};
	private:
		std::vector<const char*> GetExtensions();
		void CreateDebugMessenger();
		void CreateLogicalDevice();
		void CreateCommandBuffers();
		void CreateSyncObjects();
		
		VkPhysicalDevice PickPhysicalDevice();
		uint16_t RateDeviceSuitability(VkPhysicalDevice device);
		bool IsDeviceSuitable(VkPhysicalDevice device);
		
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	private:
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;

		QueueFamilyIndices m_QueueIndices;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device;

		VkCommandPool m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		std::vector<VkSemaphore> m_ImageSemaphores;
		std::vector<VkSemaphore> m_RenderSemaphores;
		std::vector<VkFence> m_Fences;

		std::vector<const char*> m_ValidationLayers = {
		   "VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> m_DeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
	};
}