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

		void* GetSurface() override { return m_Surface; }
	private:
		GLFWwindow* m_WindowHandle;
		VkSurfaceKHR m_Surface;
	};
}