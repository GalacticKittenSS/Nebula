#include "nbpch.h"
#include "Renderer.h"

#include "Platform/OpenGl/OpenGL_Shader.h"

namespace Nebula {
	Scope<Renderer::SceneData> Renderer::s_Data = CreateScope<Renderer::SceneData>();

	void Renderer::Init() {
		RenderCommand::Init();
	}

	void OnWindowResize(uint32_t width, uint32_t height) {
		RenderCommand::SetViewPort(0, 0, width, height);
	}

	void Renderer::BeginScene(OrthographicCamera& camera) {
		s_Data->ViewProjectMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene() {
	}

	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform) {
		shader->Bind();
		std::dynamic_pointer_cast<OpenGL_Shader>(shader)->UploadUniformMat4("view", s_Data->ViewProjectMatrix);
		std::dynamic_pointer_cast<OpenGL_Shader>(shader)->UploadUniformMat4("transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}