#include "nbpch.h"
#include "OpenGL_RendererAPI.h"

#include <glad/glad.h>

namespace Nebula {
	void OpenGL_RendererAPI::Clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGL_RendererAPI::SetClearColour(float r, float g, float b, float a) {
		glClearColor(r, g, b, a);
	}

	void OpenGL_RendererAPI::SetClearColour(const glm::vec4& colour) {
		glClearColor(colour.r, colour.g, colour.b, colour.a);
	}

	void OpenGL_RendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) {
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
}