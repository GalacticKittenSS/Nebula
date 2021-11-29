#include "nbpch.h"
#include "Renderer.h"

#include "Renderer2D.h"

namespace Nebula {
	Scope<Renderer::SceneData> Renderer::s_Data = CreateScope<Renderer::SceneData>();

	void Renderer::Init() {
		NB_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::Shutdown() {
		Renderer2D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height) {
		RenderCommand::SetViewPort(0, 0, width, height);
	}

	void Renderer::BeginScene(OrthographicCamera& camera) {
		s_Data->ViewProjectMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene() { }

	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const mat4& transform) {
		shader->Bind();
		shader->SetMat4("u_View", s_Data->ViewProjectMatrix);
		shader->SetMat4("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}