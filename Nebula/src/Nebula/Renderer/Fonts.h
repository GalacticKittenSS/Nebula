#pragma once

#include "Texture.h"
#include "Nebula/Utils/Arrays.h"

namespace ftgl {
	struct texture_atlas_t;
	struct texture_font_t;
	struct texture_glyph_t;
}

namespace Nebula {
	class FontGlyph {
	public:
		float offset_x();
		float offset_y();

		float width();
		float height();

		float s0();
		float s1();
		float t0();
		float t1();

		float advance_x();

		float GetKerning(std::string_view c) const;

		FontGlyph(ftgl::texture_glyph_t* glyph) : m_TextureGlyph(glyph)
		{
		}

		operator bool() const 
		{
			return m_TextureGlyph != nullptr;
		}

	private:
		ftgl::texture_glyph_t* m_TextureGlyph;
		friend class Font;
	};

	class Font {
	public:
		Font(std::string name, std::string filename, float resolution = 32);
		~Font();

		inline const uint32_t GetID() const;
		inline const Ref<Texture2D> GetTexture() const { return m_Texture; }

		inline const std::string& GetName() const { return m_Name; }
		inline const std::string& GetFilename() const { return m_Filename; }
		inline const float GetSize() const { return m_Resolution; }
		
		void SetScale(const vec2& scale) { m_Scale = scale; }
		const vec2& GetScale() const { return m_Scale; }

		FontGlyph GetGlyph(const char* c);
	private:
		void RecreateAtlas();
	private:
		ftgl::texture_font_t* m_FTFont = nullptr;
		ftgl::texture_atlas_t* m_FTAtlas = nullptr;

		std::unordered_map<const char*, bool> m_CharsinAtlas;

		float m_AtlasSize, m_Resolution;
		vec2 m_Scale = { 1.0f, 1.0f };
		std::string m_Name, m_Filename;
		Ref<Texture2D> m_Texture;
	};

	class FontManager {
	public:
		static void Add(Font* font);
		
		static Font* Get();
		static Font* Get(const std::string& name);
		static Font* Get(const std::string& name, uint32_t size);
		
		static void Clean();
	private:
		static Array<Font*> m_Fonts;
	};
}