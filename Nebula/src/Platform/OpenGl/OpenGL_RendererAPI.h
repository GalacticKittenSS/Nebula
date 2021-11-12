#pragma once

#include "Nebula/renderer/Renderer_API.h"

namespace Nebula {
	class OpenGL_RendererAPI: public RendererAPI {
	public:
		void Clear() override;
		void SetClearColour(float r, float g, float b, float a) override;
		void SetClearColour(const glm::vec4& colour) override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray) override;
	};
}