#pragma once

#include "Nebula/Maths/Maths.h"
#include "Vertex_Array.h"

namespace Nebula {
	class RendererAPI {
	public:
		enum class API {
			None = 0, OpenGL = 1, Vulkan = 2
		};
	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual void Shutdown() {};
		virtual void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void Clear() = 0;
		virtual void SetClearColour(float r, float g, float b, float a) = 0;
		virtual void SetClearColour(const glm::vec4& colour) = 0;

		virtual void SetBackfaceCulling(bool) = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;

		virtual void SetLineWidth(float width) = 0;
		virtual const void* GetInstance() const = 0; 
		virtual const void* GetDevice() const = 0;
		virtual const void* GetPhysicalDevice() const = 0;
		virtual const void* GetCommandBuffer(uint32_t frame) const = 0;

		inline static API GetAPI() { return s_API; }
		static Scope<RendererAPI> Create();
	private:
		static API s_API;
	};
}
