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
		void SetVsync(bool vsync) override;

		uint32_t GetImageIndex() { return m_ImageIndex; }
		uint32_t GetImageCount() { return m_SwapChainImageCount; }
		const void* GetSurface() const override { return m_Surface; }

		const VkFormat& GetImageFormat() const { return m_ImageFormat; }
		const VkImage& GetImage() const { return m_Images[m_ImageIndex]; }
		bool m_RecreateSwapChain = false;
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
		VkPresentModeKHR ChooseSwapPresentMode(VkPresentModeKHR targetPresentMode, const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		void CreateSwapChain();
		void CreateImageViews();
		void RecreateSwapChain();
		void CleanUpSwapChain();
	private:
		GLFWwindow* m_WindowHandle;
		VkSurfaceKHR m_Surface;
		VkSwapchainKHR m_SwapChain = nullptr;
		
		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;
		
		VkFormat m_ImageFormat;
		VkExtent2D m_Extent;
		VkPresentModeKHR m_PresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

		uint32_t m_ImageIndex = 0;
		uint32_t m_SwapChainImageCount;

		friend class Vulkan_FrameBuffer;
	};
}