#pragma once

#include "Nebula/renderer/Graphics_Context.h"

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace Nebula {
	class Vulkan_Context : public GraphicsContext {
	public:
		Vulkan_Context(GLFWwindow* windowHandle);

		void Init() override;
		void Shutdown() override;
		
		void SwapBuffers() override;
		
		uint32_t GetNextImage() { return m_ImageIndex; }
		uint32_t GetImageCount() { return m_SwapChainImageCount; }
		const void* GetSurface() const override { return m_Surface; }
		const void* GetImageFormat() const override { return &m_ImageFormat; }
	private:
		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};
	private:
		bool AcquireNextImage();
		void PresentCurrentImage();

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		void CreateSwapChain();
		void CreateImageViews();
		void RecreateSwapChain();
		void CleanUpSwapChain();
	private:
		GLFWwindow* m_WindowHandle;
		VkSurfaceKHR m_Surface;
		VkSwapchainKHR m_SwapChain;
		
		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;
		VkFormat m_ImageFormat;
		VkExtent2D m_Extent;
		
		uint32_t m_ImageIndex = 0;
		uint32_t m_SwapChainImageCount;

		friend class Vulkan_RendererAPI;
		friend class Vulkan_FrameBuffer;
		friend class ImGuiLayer;
	};
}