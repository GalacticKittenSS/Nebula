#pragma once

#include "Nebula/renderer/Buffer.h"

namespace Nebula {
	//-----------------------------------------------------//
	/////////////////////////////////////////////////////////
	///////////////////// Vertex BUFFER /////////////////////
	////////////////////////////////////////////////////////
	//-----------------------------------------------------//

	class OpenGL_VertexBuffer : public VertexBuffer {
	public:
		OpenGL_VertexBuffer(uint32_t size);
		OpenGL_VertexBuffer(float* vertices, uint32_t size);
		~OpenGL_VertexBuffer();

		void Bind()   const override;
		void Unbind() const override;
		void SetData(const void* data, uint32_t size = 0) override;

		const BufferLayout GetLayout() const override { return m_Layout; }
		void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;

	};

	//----------------------------------------------------//
	////////////////////////////////////////////////////////
	///////////////////// INDEX BUFFER /////////////////////
	////////////////////////////////////////////////////////
	//----------------------------------------------------//

	class OpenGL_IndexBuffer : public IndexBuffer {
	public:
		OpenGL_IndexBuffer(uint32_t* indices, uint32_t count);
		~OpenGL_IndexBuffer();

		void Bind()   const override;
		void Unbind() const override;

		uint32_t GetCount() const override { return m_Count; }
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
}