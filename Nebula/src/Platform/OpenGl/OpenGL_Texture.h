#pragma once

#include "Nebula/renderer/Texture.h"

namespace Nebula {
	class OpenGL_Texture2D : public Texture2D {
	public:
		OpenGL_Texture2D(const std::string& path);
		~OpenGL_Texture2D();
		
		uint32_t GetWidth() const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }

		void Bind(uint32_t slot) const;
	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
	};
}
