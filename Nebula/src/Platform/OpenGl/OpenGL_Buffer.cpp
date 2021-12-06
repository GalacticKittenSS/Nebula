#include "nbpch.h"
#include "OpenGL_Buffer.h"

#include <glad/glad.h>

namespace Nebula {
	//-----------------------------------------------------//
	/////////////////////////////////////////////////////////
	///////////////////// Vertex BUFFER /////////////////////
	////////////////////////////////////////////////////////
	//-----------------------------------------------------//
	
	OpenGL_VertexBuffer::OpenGL_VertexBuffer(uint32_t size) {
		NB_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	
	OpenGL_VertexBuffer::OpenGL_VertexBuffer(float* vertices, uint32_t size) {
		NB_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	OpenGL_VertexBuffer::~OpenGL_VertexBuffer() {
		NB_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGL_VertexBuffer::Bind() const {
		NB_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGL_VertexBuffer::Unbind() const {
		NB_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}


	void OpenGL_VertexBuffer::SetData(const void* data, uint32_t size) {
		NB_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}
	
	//----------------------------------------------------//
	////////////////////////////////////////////////////////
	///////////////////// INDEX BUFFER /////////////////////
	////////////////////////////////////////////////////////
	//----------------------------------------------------//

	OpenGL_IndexBuffer::OpenGL_IndexBuffer(uint32_t* indices, uint32_t count): m_Count(count) {
		NB_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

	OpenGL_IndexBuffer::~OpenGL_IndexBuffer() {
		NB_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGL_IndexBuffer::Bind() const {
		NB_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGL_IndexBuffer::Unbind() const {
		NB_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}