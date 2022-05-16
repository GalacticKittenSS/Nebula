#pragma once

#include "Nebula/renderer/Vertex_Array.h"

namespace Nebula {
	class OpenGL_VertexArray : public VertexArray {
	public:
		OpenGL_VertexArray();
		~OpenGL_VertexArray();

		void Bind() const override;
		void Unbind() const override;

		void AddVertexBuffer(const Ref<VertexBuffer>& buffer) override;
		void SetIndexBuffer(const Ref<IndexBuffer>& buffer) override;

		const Array<Ref<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
		const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
	private:
		uint32_t m_RendererID;
		uint32_t m_VertexBufferIndex = 0;
		Array<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
	};
}