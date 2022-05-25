#pragma once

#include "Texture.h"
#include "Nebula/Utils/Arrays.h"

namespace ftgl {
	class texture_atlas_t;
	class texture_font_t;
	class texture_glyph_t;
}

namespace Nebula {
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

		ftgl::texture_glyph_t* GetGlyph(const char* c);
		float GetGlyphKerning(const ftgl::texture_glyph_t* glyph, const char* c) const;
	private:
		void RecreateAtlas(bool firstTime = false);
	private:
		ftgl::texture_font_t* m_FTFont;
		ftgl::texture_atlas_t* m_FTAtlas;

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