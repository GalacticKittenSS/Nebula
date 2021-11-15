#include "nbpch.h"
#include "OpenGL_Texture.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace Nebula {
	OpenGL_Texture2D::OpenGL_Texture2D(uint32_t width, uint32_t height) : m_Width(width), m_Height(height) {
		m_InternalFormat = GL_RGBA8;
		m_Format = GL_RGBA;
		
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGL_Texture2D::OpenGL_Texture2D(const std::string& path): m_Path(path) {
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		NB_ASSERT(data, "Failed to Load Image!");
		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4) {
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3) {
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		m_InternalFormat = internalFormat;
		m_Format = dataFormat;

		NB_ASSERT(internalFormat & dataFormat, "[Texture] Format Not Supported!");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	OpenGL_Texture2D::~OpenGL_Texture2D() {
		glDeleteTextures(1, &m_RendererID);
	}
	
	void OpenGL_Texture2D::SetData(void* data, uint32_t size) {
		uint32_t bpp = m_Format == GL_RGBA ? 4 : 3;
		NB_ASSERT(size == m_Width * m_Height * bpp, "Data must be Entire Texture");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_Format, GL_UNSIGNED_BYTE, data);
	}

	void OpenGL_Texture2D::Bind(uint32_t slot = 0) const {
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGL_Texture2D::Unbind() const {
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}