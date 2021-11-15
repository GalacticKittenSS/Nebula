#pragma once

#include "Nebula/renderer/Renderer_API.h"

namespace Nebula {
	class OpenGL_RendererAPI: public RendererAPI {
	public:
		void Init() override;
		void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void Clear() override;
		void SetClearColour(float r, float g, float b, float a) override;
		void SetClearColour(const glm::vec4& colour) override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray) override;
	};
}