#include "nbpch.h"
#include "SceneRenderer.h"

#include "Nebula/Core/Application.h"
#include "Nebula/Renderer/Render_Command.h"
#include "Nebula/Renderer/Framebuffer.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include <map>
#include <optional>
#include <set>

namespace Nebula
{
	struct VulkanData
	{
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		
		Ref<Shader> shader;
		Ref<FrameBuffer> frambuffer;
		
		uint32_t currentFrame = 0;
		bool framebufferResized = false;
	};
	static VulkanData s_VKData;

	SceneRenderer::Settings SceneRenderer::m_Settings = {};

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		s_VKData.framebufferResized = true;
	}

	void SceneRenderer::Setup()
	{
		Window& window = Application::Get().GetWindow();

		FrameBufferSpecification spec;
		spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::DEPTH24STENCIL8 };
		spec.Width = window.GetWidth();
		spec.Height = window.GetHeight();
		spec.SwapChainTarget = true;

		s_VKData.frambuffer = FrameBuffer::Create(spec);
		s_VKData.frambuffer->Bind();
		s_VKData.shader = Shader::Create("Resources/shaders/Vulkan.glsl");
	}

	void** SceneRenderer::GetGraphicsQueue()
	{
		return (void**)&s_VKData.graphicsQueue;
	}

	void** SceneRenderer::GetPresentQueue()
	{
		return (void**)&s_VKData.presentQueue;
	}

	Ref<Shader> SceneRenderer::GetShader()
	{
		return s_VKData.shader;
	}

	void SceneRenderer::Render()
	{
		s_VKData.frambuffer->Bind();
		RenderCommand::DrawIndexed(nullptr);
		s_VKData.frambuffer->Unbind();
	}

	void SceneRenderer::CleanUp()
	{
	}
}