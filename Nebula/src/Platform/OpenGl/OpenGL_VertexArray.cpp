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
		NB_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGL_VertexArray::~OpenGL_VertexArray() {
		NB_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGL_VertexArray::Bind() const {
		NB_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
	}

	void OpenGL_VertexArray::Unbind() const {
		NB_PROFILE_FUNCTION();

		glBindVertexArray(0);
	}

	void OpenGL_VertexArray::AddVertexBuffer(const Ref<VertexBuffer>& buffer) {
		NB_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
		buffer->Bind();

		const auto& layout = buffer->GetLayout();
		for (const auto& element : layout) {
			switch (element.Type) {
				case ShaderDataType::Float:
				case ShaderDataType::Float2:
				case ShaderDataType::Float3:
				case ShaderDataType::Float4:
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(
						m_VertexBufferIndex,
						element.GetComponentCount(),
						ShaderTypetoOpenGL(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)(intptr_t)element.Offset
					);
					m_VertexBufferIndex++;
					break;
				}

				case ShaderDataType::Int:
				case ShaderDataType::Int2:
				case ShaderDataType::Int3:
				case ShaderDataType::Int4:
				case ShaderDataType::Bool:
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribIPointer(
						m_VertexBufferIndex,
						element.GetComponentCount(),
						ShaderTypetoOpenGL(element.Type),
						layout.GetStride(),
						(const void*)(intptr_t)element.Offset
					);
					m_VertexBufferIndex++;
					break;
				}
				
				case ShaderDataType::Mat3:
				case ShaderDataType::Mat4:
				{
					uint8_t count = element.GetComponentCount();
					for (uint8_t i = 0; i < count; i++)
					{
						glEnableVertexAttribArray(m_VertexBufferIndex);
						glVertexAttribPointer(
							m_VertexBufferIndex,
							count,
							ShaderTypetoOpenGL(element.Type),
							element.Normalized ? GL_TRUE : GL_FALSE,
							layout.GetStride(),
							(const void*)(sizeof(float) * count * i));
						glVertexAttribDivisor(m_VertexBufferIndex, 1);
						m_VertexBufferIndex++;
					}
					break;
				}
				default:
					NB_ASSERT(false, "Unknown ShaderDataType!");
			}
		}

		m_VertexBuffers.push_back(buffer);
	}

	void OpenGL_VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& buffer) {
		NB_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
		buffer->Bind();

		m_IndexBuffer = buffer;
	}
}