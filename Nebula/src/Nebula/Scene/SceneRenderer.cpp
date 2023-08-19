#include "nbpch.h"
#include "SceneRenderer.h"

#include "Nebula/Core/Application.h"
#include "Nebula/Renderer/Render_Command.h"
#include "Nebula/Renderer/Framebuffer.h"
#include "Nebula/Renderer/UniformBuffer.h"
#include "Nebula/Renderer/Renderer2D.h"
#include "Nebula/AssetManager/TextureImporter.h"

#include <map>
#include <optional>
#include <set>

namespace Nebula
{
	struct VulkanData
	{
		Ref<FrameBuffer> frambuffer;
		bool framebufferResize = false;
		uint32_t width = 0, height = 0;

		Ref<Texture2D> Texture;
		EditorCamera camera;
	};
	static VulkanData s_VKData;

	SceneRenderer::Settings SceneRenderer::m_Settings = {};

	void SceneRenderer::Setup()
	{
		Window& window = Application::Get().GetWindow();

		FrameBufferSpecification spec;
		spec.Attachments = { AttachmentTextureFormat::RGBA8, AttachmentTextureFormat::RED_INT, AttachmentTextureFormat::DEPTH24STENCIL8 };
		spec.Width = window.GetWidth();
		spec.Height = window.GetHeight();
		spec.SwapChainTarget = true;

		s_VKData.frambuffer = FrameBuffer::Create(spec);
		
		// Shader currently needs VkRenderPass object located in framebuffer
		// without a framebuffer bound vulkan will throw errors
		Renderer2D::Init();

		s_VKData.Texture = TextureImporter::CreateTexture2D("Resources/Vulkan/Texture.jpg");
		s_VKData.camera = EditorCamera(60.0f, 16.0f / 9.0f, 0.01f, 1000.0f);
		s_VKData.camera.SetViewPortSize((float)window.GetWidth(), (float)window.GetHeight());

		RenderCommand::SetClearColour({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::SetBackfaceCulling(false);
		RenderCommand::SetLineWidth(0.5f);
	}

	void SceneRenderer::OnEvent(Event& e)
	{
		s_VKData.camera.OnEvent(e);
	}

	bool SceneRenderer::OnWindowResize(WindowResizeEvent& e)
	{
		s_VKData.framebufferResize = true;
		s_VKData.width = e.GetWidth();
		s_VKData.height = e.GetHeight(); 
		s_VKData.camera.SetViewPortSize((float)e.GetWidth(), (float)e.GetHeight());
		return false;
	}

	void SceneRenderer::Render()
	{
		if (s_VKData.framebufferResize)
		{
			s_VKData.frambuffer->Resize(s_VKData.width, s_VKData.height);
			s_VKData.framebufferResize = false;
		}

		s_VKData.camera.Update();

		auto currentTime = std::chrono::high_resolution_clock::now();
		
		s_VKData.frambuffer->Bind();
		s_VKData.frambuffer->ClearAttachment(1, -1);
		RenderCommand::Clear();

		Renderer2D::BeginScene(s_VKData.camera);
		
		const glm::vec4 vertices[] = {
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f,  0.5f, 0.0f, 1.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f }
		}; 

		glm::vec2 texCoords[] = {
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f }
		}; 

		Material mat;
		mat.Texture = s_VKData.Texture;

		Renderer2D::DrawQuad(4, vertices, texCoords, glm::mat4(1.0f), mat);
		Renderer2D::EndScene();
		
		Renderer2D::BeginScene(s_VKData.camera);
		
		const glm::vec4 vertices2[] = {
			{  0.5f,  0.5f, 0.0f, 1.0f },
			{  1.5f,  0.5f, 0.0f, 1.0f },
			{  1.5f,  1.5f, 0.0f, 1.0f },
			{  0.5f,  1.5f, 0.0f, 1.0f }
		}; 

		Renderer2D::DrawQuad(4, vertices2, texCoords, glm::mat4(1.0f), mat);
		Renderer2D::EndScene();
		s_VKData.frambuffer->Unbind();
		
		// Frame Counter
		{
			static auto lastUpdateTime = std::chrono::high_resolution_clock::now();
			float timeSinceUpdate = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastUpdateTime).count();

			static int frames = 0;
			frames++;

			if (timeSinceUpdate >= 1.0f)
			{
				NB_INFO(frames);
				lastUpdateTime = std::chrono::high_resolution_clock::now();
				frames = 0;
			}
		}
	}

	void SceneRenderer::CleanUp()
	{
	}
}