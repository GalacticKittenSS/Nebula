#pragma once

#include "Texture.h"

namespace ftgl {
	class texture_atlas_t;
	class texture_font_t;
	class texture_glyph_t;
}

namespace Nebula {
	class Font {
	public:
		Font(std::string name, std::string filename, float resolution = 32);
		~Font() = default;

		inline const uint32_t getID() const;
		inline const Ref<Texture2D> getTexture() const { return m_Texture; }

		inline const std::string& getName() const { return m_Name; }
		inline const std::string& getFilename() const { return m_Filename; }
		inline const float getSize() const { return m_Resolution; }

		void SetScale(const vec2& scale) { m_Scale = scale; }
		const vec2& GetScale() const { return m_Scale; }

		ftgl::texture_glyph_t* getGlyph(const char* c);
		float getGlyphKerning(const ftgl::texture_glyph_t* glyph, const char* c) const;
	private:
		ftgl::texture_font_t* m_FTFont;
		ftgl::texture_atlas_t* m_FTAtlas;

		std::unordered_map<const char*, bool> m_CharsinAtlas;

		float m_AtlasSize, m_Resolution;
		vec2 m_Scale = { 1.0f, 1.0f };
		std::string m_Name, m_Filename;
		Ref<Texture2D> m_Texture;
	};
}