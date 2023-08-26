#include "nbpch.h"
#include "OpenGL_Texture.h"

#include "OpenGL_Image.h"

#include <glad/glad.h>

namespace Nebula {
	namespace Utils {
		uint32_t OpenGLtoBPP(GLenum format) {
			switch (format)
			{
			case GL_RED: return 1;
			case GL_RGB: return 3;
			case GL_RGBA: return 4;
			}

			NB_ASSERT(false, "Unknown OpenGL Format");
			return 0;
		}

		static GLenum NebulaToGLDataFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RGB8:		return GL_RGB;
				case ImageFormat::RGBA8:	return GL_RGBA;
			}

			NB_ASSERT(false);
			return 0;
		}

		static GLenum NebulaToGLInternalFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RGB8:		return GL_RGB8;
				case ImageFormat::RGBA8:	return GL_RGBA8;
			}

			NB_ASSERT(false);
			return 0;
		}
	}

	OpenGL_Texture2D::OpenGL_Texture2D(const TextureSpecification& specification, Buffer data)
		: m_Specification(specification), m_Width(specification.Width), m_Height(specification.Height)
	{
		NB_PROFILE_FUNCTION();

		m_InternalFormat = Utils::NebulaToGLInternalFormat(specification.Format);
		m_Format = Utils::NebulaToGLDataFormat(specification.Format);
		
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		m_Image = CreateRef<OpenGL_Image>(m_RendererID);

		if (data)
		{
			SetData(data);
			m_IsLoaded = true;
		}
	}

	OpenGL_Texture2D::~OpenGL_Texture2D() {
		NB_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_RendererID);
	}
	
	void OpenGL_Texture2D::SetData(Buffer data) {
		NB_PROFILE_FUNCTION();

		uint32_t bpp = Utils::OpenGLtoBPP(m_Format);
		NB_ASSERT(data.Size == m_Width * m_Height * bpp, "Data must be Entire Texture");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_Format, GL_UNSIGNED_BYTE, data.Data);
	}

	void OpenGL_Texture2D::SetFilterNearest(bool nearest) {
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, nearest ? GL_NEAREST : GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, nearest ? GL_NEAREST : GL_LINEAR);
	}

	void OpenGL_Texture2D::Bind(uint32_t slot = 0) const {
		NB_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGL_Texture2D::Unbind() const {
		NB_PROFILE_FUNCTION();

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}