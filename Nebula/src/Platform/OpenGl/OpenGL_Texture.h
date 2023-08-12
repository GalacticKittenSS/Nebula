#pragma once

#include "Nebula/renderer/Texture.h"

#include <glad/glad.h>

namespace Nebula {
	class OpenGL_Texture2D : public Texture2D {
	public:
		OpenGL_Texture2D(const TextureSpecification& specification, Buffer data = Buffer());
		~OpenGL_Texture2D();

		const TextureSpecification& GetSpecification() const override { return m_Specification; }

		void SetData(Buffer data) override;
		void SetFilterNearest(bool nearest) override;
		
		uint32_t GetWidth() const override { return m_Width; }
		uint32_t GetHeight() const override { return m_Height; }
		uint64_t GetRendererID() const override { return m_RendererID; }
		
		void Bind(uint32_t slot) const;
		void Unbind() const;

		bool IsLoaded() const override { return m_IsLoaded; }

		bool operator==(const Texture& other) const override {
			return m_RendererID == other.GetRendererID();
		}
	private:
		TextureSpecification m_Specification;

		bool m_IsLoaded = false;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_Format;
	};
}
