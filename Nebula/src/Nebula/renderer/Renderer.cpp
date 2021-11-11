#include "nbpch.h"
#include "Renderer.h"

namespace Nebula {
	Renderer::SceneData* Renderer::m_Data = new Renderer::SceneData();

	void Renderer::BeginScene(OrthographicCamera& camera) {
		m_Data->ViewProjectMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene() {
	}

	void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray) {
		shader->Bind();
		shader->UploadUniformMat4("view", m_Data->ViewProjectMatrix);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}
}