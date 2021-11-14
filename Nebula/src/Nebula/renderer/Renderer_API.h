#pragma once

#include <glm/glm.hpp>

#include "Vertex_Array.h"

namespace Nebula {
	class RendererAPI {
	public:
		enum class API {
			None = 0, OpenGL = 1
		};
	public:
		virtual void Init() = 0;

		virtual void Clear() = 0;
		virtual void SetClearColour(float r, float g, float b, float a) = 0;
		virtual void SetClearColour(const glm::vec4& colour) = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) = 0;

		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};
}