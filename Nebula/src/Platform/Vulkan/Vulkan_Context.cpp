#include "nbpch.h"
#include "Vulkan_Context.h"

#include "Nebula/Renderer/Render_Command.h"

#include <GLFW/glfw3.h>

namespace Nebula {
	Vulkan_Context::Vulkan_Context(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle) {
		NB_ASSERT(windowHandle, "Window Handle is NULL!")
	}

	static std::string VendorIDToString(uint32_t vendor)
	{
		switch (vendor)
		{
		case 0x1002: return "AMD";
		case 0x1010: return "ImgTec";
		case 0x10DE: return "NVIDIA";
		case 0x13B5: return "ARM";
		case 0x5143: return "Qualcomm";
		case 0x8086: return "INTEL";
		}
	}

	void Vulkan_Context::Init() {
		NB_PROFILE_FUNCTION();

		VkResult result = glfwCreateWindowSurface((VkInstance)RenderCommand::GetInstance(), m_WindowHandle, nullptr, &m_Surface);
		NB_ASSERT(result == VK_SUCCESS, "Failed to create window surface");

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties((VkPhysicalDevice)RenderCommand::GetPhysicalDevice(), &properties);

		uint8_t versionMajor = VK_API_VERSION_MAJOR(properties.apiVersion);
		uint8_t versionMinor = VK_API_VERSION_MINOR(properties.apiVersion);
		uint8_t versionPatch = VK_API_VERSION_PATCH(properties.apiVersion);

		NB_INFO("Vulkan Info:");
		NB_INFO("  Vendor: {0}", VendorIDToString(properties.vendorID));
		NB_INFO("  Renderer: {0}", properties.deviceName);
		NB_INFO("  Version: {0}.{1}.{2}", versionMajor, versionMinor, versionPatch);

		NB_ASSERT(versionMajor == 1 && versionMinor >= 3, "Nebula Requires Vulkan version 1.3 and Up!");
	}

	void Vulkan_Context::Shutdown()
	{
		NB_PROFILE_FUNCTION();
		vkDestroySurfaceKHR((VkInstance)RenderCommand::GetInstance(), m_Surface, nullptr);
	}

	void Vulkan_Context::SwapBuffers() 
	{
	}
}