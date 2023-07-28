#include "nbpch.h"
#include "SceneRenderer.h"

#include "Nebula/Core/Application.h"
#include "Nebula/Renderer/Render_Command.h"
#include "Nebula/Renderer/Framebuffer.h"



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

		Ref<VertexBuffer> vBuffer;
		Ref<IndexBuffer> iBuffer;
		Ref<VertexArray> vao;
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

		const float vertices[] = {
			-0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
			 0.5f, -0.5f, 1.0f, 1.0f, 1.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		const uint32_t indices[] = {
			0, 1, 2, 2, 3, 0
		};

		s_VKData.vBuffer = VertexBuffer::Create((float*)vertices, sizeof(vertices));
		s_VKData.iBuffer = IndexBuffer::Create((uint32_t*)indices, 6);
		
		s_VKData.vao = VertexArray::Create();
		s_VKData.vao->AddVertexBuffer(s_VKData.vBuffer);
		s_VKData.vao->SetIndexBuffer(s_VKData.iBuffer);
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
		RenderCommand::DrawIndexed(s_VKData.vao);
		//s_VKData.frambuffer->Unbind();
	}

	void SceneRenderer::CleanUp()
	{
	}
}