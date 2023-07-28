#pragma once

#include "Nebula/Renderer/Vertex_Array.h"

namespace Nebula {
	class Vulkan_VertexArray : public VertexArray {
	public:
		Vulkan_VertexArray();
		~Vulkan_VertexArray();

		void Bind() const override;
		void Unbind() const override;

		void AddVertexBuffer(const Ref<VertexBuffer>& buffer) override;
		void SetIndexBuffer(const Ref<IndexBuffer>& buffer) override;

		const Array<Ref<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
		const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
	private:
		uint32_t m_VertexBufferIndex = 0;
		Array<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
	};
}