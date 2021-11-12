#pragma once

#include "Nebula/renderer/Vertex_Array.h"

namespace Nebula {
	class OpenGL_VertexArray : public VertexArray {
	public:
		OpenGL_VertexArray();
		~OpenGL_VertexArray();

		void Bind() const override;
		void Unbind() const override;

		void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& buffer) override;
		void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& buffer) override;

		const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
		const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
	private:
		uint32_t m_RendererID;
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
	};
}