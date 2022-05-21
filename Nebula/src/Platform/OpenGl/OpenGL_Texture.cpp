#include "nbpch.h"
#include "OpenGL_Texture.h"

#include <glad/glad.h>
#include <stb_image.h>

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
		}
	}

	OpenGL_Texture2D::OpenGL_Texture2D(uint32_t width, uint32_t height, bool alphaOnly) : m_Width(width), m_Height(height) {
		NB_PROFILE_FUNCTION();

		m_InternalFormat = GL_RGBA8;
		m_Format = GL_RGBA;
		
		if (alphaOnly) {
			m_InternalFormat = GL_R8;
			m_Format = GL_RED;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		if (alphaOnly) {
			GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };

			glBindTexture(GL_TEXTURE_2D, m_RendererID);
			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	OpenGL_Texture2D::OpenGL_Texture2D(const std::string& path): m_Path(path) {
		NB_PROFILE_FUNCTION();

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		
		if (!data) return;

		m_IsLoaded = true;

		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		m_InternalFormat = internalFormat;
		m_Format = dataFormat;

		NB_ASSERT(internalFormat & dataFormat, "[Texture] Format Not Supported!");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	OpenGL_Texture2D::~OpenGL_Texture2D() {
		NB_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_RendererID);
	}
	
	void OpenGL_Texture2D::SetData(void* data, uint32_t size) {
		NB_PROFILE_FUNCTION();

		uint32_t bpp = Utils::OpenGLtoBPP(m_Format);
		NB_ASSERT(size == m_Width * m_Height * bpp, "Data must be Entire Texture");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_Format, GL_UNSIGNED_BYTE, data);
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