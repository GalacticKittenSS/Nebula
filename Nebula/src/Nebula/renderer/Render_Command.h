#pragma once

#include "Renderer_API.h"

namespace Nebula {
	class RenderCommand {
	public:
		inline static void Clear() {
			s_RendererAPI->Clear();
		}

		inline static void SetClearColour(float r, float g, float b, float a) {
			s_RendererAPI->SetClearColour(r, g, b, a);
		}

		inline static void SetClearColour(const glm::vec4& colour) {
			s_RendererAPI->SetClearColour(colour);
		}

		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) {
			s_RendererAPI->DrawIndexed(vertexArray);
		}
	private:
		static RendererAPI* s_RendererAPI;
	};
}