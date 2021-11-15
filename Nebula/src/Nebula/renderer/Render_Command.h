#pragma once

#include "Renderer_API.h"

namespace Nebula {
	class RenderCommand {
	public:
		inline static void Init() {
			s_RendererAPI->Init();
		}

		inline static void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
			s_RendererAPI->SetViewPort(x ,y, width, height);
		}

		inline static void Clear() {
			s_RendererAPI->Clear();
		}

		inline static void SetClearColour(float r, float g, float b, float a) {
			s_RendererAPI->SetClearColour(r, g, b, a);
		}

		inline static void SetClearColour(const glm::vec4& colour) {
			s_RendererAPI->SetClearColour(colour);
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray) {
			s_RendererAPI->DrawIndexed(vertexArray);
		}
	private:
		static Scope<RendererAPI> s_RendererAPI;
	};
}