#pragma once

#include "Nebula/renderer/Renderer_API.h"

#include <vulkan/vulkan.h>
#include <optional>

#include "Vulkan_Context.h"

namespace Nebula {
	class Vulkan_RendererAPI : public RendererAPI {
	public:
		void Init() override;
		void Shutdown() override;
		void SetViewPort(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void Clear() override;
		void SetClearColour(float r, float g, float b, float a) override;
		void SetClearColour(const glm::vec4& colour) override;

		void SetBackfaceCulling(bool) override;

		void BeginRecording() override;
		void EndRecording() override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) override;
		void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;

		void SetLineWidth(float width) override;
	private:
		void CreateDebugMessenger();
		void recordCommandBuffer(Ref<VertexArray> array, VkCommandBuffer commandBuffer, uint32_t imageIndex);
	private:
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		glm::vec4 m_ClearColour;
		float m_LineWidth = 1.0f;
		bool m_BackFaceCulling = false;
		
		friend class Vulkan_Context;
	};
}