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
		Ref<Shader> shader;
		Ref<FrameBuffer> frambuffer;

		bool framebufferResize = false;
		uint32_t width = 0, height = 0;
	};
	static VulkanData s_VKData;

	SceneRenderer::Settings SceneRenderer::m_Settings = {};

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

	Ref<Shader> SceneRenderer::GetShader()
	{
		return s_VKData.shader;
	}

	bool SceneRenderer::OnWindowResize(WindowResizeEvent& e)
	{
		s_VKData.framebufferResize = true;
		s_VKData.width = e.GetWidth();
		s_VKData.height = e.GetHeight(); 
		return false;
	}

	void SceneRenderer::Render()
	{
		if (s_VKData.framebufferResize)
		{
			s_VKData.frambuffer->Resize(s_VKData.width, s_VKData.height);
			s_VKData.framebufferResize = false;
		}

		s_VKData.frambuffer->Bind();
		RenderCommand::DrawIndexed(nullptr);
		//s_VKData.frambuffer->Unbind();
	}

	void SceneRenderer::CleanUp()
	{
	}
}