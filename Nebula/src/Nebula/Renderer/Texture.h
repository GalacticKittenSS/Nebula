#pragma once

#include <string>

#include "Nebula/Core/API.h"
#include "Nebula/Maths/Maths.h"

namespace Nebula {
	enum class ImageFormat
	{
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RGBA32F
	};

	struct TextureSpecification
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		ImageFormat Format = ImageFormat::RGBA8;
		bool GenerateMips = true;
	};

	class Texture {
	public:
		virtual ~Texture() = default;

		virtual const TextureSpecification& GetSpecification() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual std::string GetPath() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void SetFilterNearest(bool nearest = true) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void Unbind() const = 0;

		virtual bool IsLoaded() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture {
	public:
		static Ref<Texture2D> Create(const TextureSpecification& specification);
		static Ref<Texture2D> Create(const std::string& path);
	};

	class SubTexture2D {
	public:
		SubTexture2D(const Ref<Texture2D>& texture, glm::vec2& min, glm::vec2& max);
		~SubTexture2D();

		const Ref<Texture2D> GetTexture() const { return m_Texture; }
		const glm::vec2* GetTextureCoords() const  { return m_TexCoords; }
		glm::vec2* GetTextureCoords() { return m_TexCoords; }

		static Ref<SubTexture2D> CreateFromCoords(const Ref<Texture2D>& texture, const glm::vec2& coords,
			const glm::vec2& cellSize, const glm::vec2& spriteSize = { 1, 1 });
		static Ref<SubTexture2D> CreateFromCoords(const std::string& path, const glm::vec2& coords,
			const glm::vec2& cellSize, const glm::vec2& spriteSize = { 1, 1 });
	private:
		Ref<Texture2D> m_Texture;

		glm::vec2* m_TexCoords;
	};
}
