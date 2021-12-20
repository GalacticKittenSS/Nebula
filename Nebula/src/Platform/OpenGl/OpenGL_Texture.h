#pragma once

#include "Nebula/renderer/Texture.h"

#include <glad/glad.h>

namespace Nebula {
	class OpenGL_Texture2D : public Texture2D {
	public:
		OpenGL_Texture2D(const std::string& path);
		OpenGL_Texture2D(uint32_t width, uint32_t height);
		~OpenGL_Texture2D();

		void SetData(void* data, uint32_t size) override;
		
		uint32_t GetWidth() const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }
		uint32_t GetRendererID() const override { return m_RendererID; }
		std::string GetPath() const override { return m_Path; }

		void Bind(uint32_t slot) const;
		void Unbind() const;

		bool IsLoaded() const override { return m_IsLoaded; }

		bool operator==(const Texture& other) const override { 
			return m_RendererID == ((OpenGL_Texture2D&)other).m_RendererID;
		}
	private:
		std::string m_Path;
		bool m_IsLoaded = false;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_Format;
	};
}
