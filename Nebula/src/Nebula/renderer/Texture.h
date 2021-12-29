#pragma once

#include <string>

#include "Nebula/Core/API.h"
#include "Nebula/Maths/Maths.h"

namespace Nebula {
	class Texture {
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual std::string GetPath() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void Unbind() const = 0;

		virtual bool IsLoaded() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture {
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(const std::string& path);
	};

	class SubTexture2D {
	public:
		SubTexture2D(const Ref<Texture2D>& texture, vec2& min, vec2& max);

		const Ref<Texture2D> GetTexture() const { return m_Texture; }
		const vec2* GetTextureCoords() const  { return m_TexCoords; }
		vec2* GetTextureCoords() { return m_TexCoords; }

		static Ref<SubTexture2D> CreateFromCoords(const Ref<Texture2D>& texture, const vec2& coords, 
			const vec2& cellSize, const vec2& spriteSize = { 1, 1 });
		static Ref<SubTexture2D> CreateFromCoords(const std::string& path, const vec2& coords, 
			const vec2& cellSize, const vec2& spriteSize = { 1, 1 });
	private:
		Ref<Texture2D> m_Texture;

		vec2 m_TexCoords[4];
	};
}