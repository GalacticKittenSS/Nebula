#include "nbpch.h"
#include "SceneRenderer.h"

#include "Nebula/Core/Application.h"
#include "Nebula/Renderer/Render_Command.h"
#include "Nebula/Renderer/Framebuffer.h"
#include "Nebula/Renderer/UniformBuffer.h"



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

		Ref<UniformBuffer> CameraUniformBuffer;
	};
	static VulkanData s_VKData;

	struct UniformBufferObject 
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	SceneRenderer::Settings SceneRenderer::m_Settings = {};

	void SceneRenderer::Setup()
	{
		Window& window = Application::Get().GetWindow();

		FrameBufferSpecification spec;
		spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INT, FramebufferTextureFormat::DEPTH24STENCIL8 };
		spec.Width = window.GetWidth();
		spec.Height = window.GetHeight();
		spec.SwapChainTarget = true;

		s_VKData.frambuffer = FrameBuffer::Create(spec);
		s_VKData.frambuffer->Bind();
		s_VKData.shader = Shader::Create("Resources/shaders/Vulkan.glsl");

		const float vertices[] = {
			-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
			 0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
		};

		const uint32_t indices[] = {
			0, 1, 2, 2, 3, 0
		};

		s_VKData.vBuffer = VertexBuffer::Create((float*)vertices, sizeof(vertices));
		s_VKData.iBuffer = IndexBuffer::Create((uint32_t*)indices, 6);
		
		s_VKData.vao = VertexArray::Create();
		s_VKData.vao->AddVertexBuffer(s_VKData.vBuffer);
		s_VKData.vao->SetIndexBuffer(s_VKData.iBuffer);
		
		s_VKData.CameraUniformBuffer = UniformBuffer::Create(sizeof(UniformBufferObject), 0);

		RenderCommand::SetBackfaceCulling(true);
		RenderCommand::SetLineWidth(0.5f);
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

		static auto startTime = std::chrono::high_resolution_clock::now();
		
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		Window& window = Application::Get().GetWindow();

		UniformBufferObject ubo{};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), window.GetWidth() / (float)window.GetHeight(), 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		s_VKData.CameraUniformBuffer->SetData(&ubo, sizeof(ubo));

		s_VKData.frambuffer->Bind();
		s_VKData.frambuffer->ClearAttachment(1, -1);

		RenderCommand::Clear();
		RenderCommand::DrawIndexed(s_VKData.vao);
		
		s_VKData.frambuffer->Unbind();
	}

	void SceneRenderer::CleanUp()
	{
	}
}