#include "nbpch.h"
#include "OpenGL_VertexArray.h"

#include <glad/glad.h>

namespace Nebula {
	static GLenum ShaderTypetoOpenGL(ShaderDataType type) {
		switch (type) {
		case ShaderDataType::Float:		return GL_FLOAT;
		case ShaderDataType::Float2:	return GL_FLOAT;
		case ShaderDataType::Float3:	return GL_FLOAT;
		case ShaderDataType::Float4:	return GL_FLOAT;
		case ShaderDataType::Int:		return GL_INT;
		case ShaderDataType::Int2:		return GL_INT;
		case ShaderDataType::Int3:		return GL_INT;
		case ShaderDataType::Int4:		return GL_INT;
		case ShaderDataType::Mat3:		return GL_FLOAT;
		case ShaderDataType::Mat4:		return GL_FLOAT;
		case ShaderDataType::Bool:		return GL_BOOL;
		}

		NB_ASSERT(false, "Unknow Shader Data Type!");
		return 0;
	}


	OpenGL_VertexArray::OpenGL_VertexArray() {
		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGL_VertexArray::~OpenGL_VertexArray() {
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGL_VertexArray::Bind() const {
		glBindVertexArray(m_RendererID);
	}

	void OpenGL_VertexArray::Unbind() const {
		glBindVertexArray(0);
	}

	void OpenGL_VertexArray::AddVertexBuffer(const Ref<VertexBuffer>& buffer) {
		//NB_ASSERT(buffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		glBindVertexArray(m_RendererID);
		buffer->Bind();

		uint32_t index = 0;
		const auto& layout = buffer->GetLayout();
		for (const auto& element : layout) {
			glEnableVertexAttribArray(index);

			glVertexAttribPointer(
				index,
				element.GetComponentCount(),
				ShaderTypetoOpenGL(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset
			);

			index++;
		}

		m_VertexBuffers.push_back(buffer);

	}

	void OpenGL_VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& buffer) {
		glBindVertexArray(m_RendererID);
		buffer->Bind();

		m_IndexBuffer = buffer;
	}
}