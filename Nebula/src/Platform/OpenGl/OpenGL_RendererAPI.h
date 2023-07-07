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
		
		void SetBackfaceCulling(bool) override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) override;
		void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;

		void SetLineWidth(float width) override;
		const void* GetInstance() const override { return nullptr; }
		const void* GetDevice() const override { return nullptr; }
		const void* GetPhysicalDevice() const override { return nullptr; }
		const void* GetCommandBuffer(uint32_t frame) const override { return nullptr; }
	};
}