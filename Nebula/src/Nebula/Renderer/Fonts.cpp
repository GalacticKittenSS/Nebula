#include "nbpch.h"
#include "Fonts.h"

#define GL_WITH_GLAD
#define NOT_USING_FT_GL_NAMESPACE
#include <freetype-gl.h>

namespace Nebula {
	Font::Font(std::string name, std::string filename, float resolution) 
		: m_Name(name), m_Filename(filename), m_Resolution(resolution)
	{
		const size_t size = (const size_t)m_Resolution * 16.0f;
		m_AtlasSize = size * size * 1.0f;

		m_FTAtlas = ftgl::texture_atlas_new(size, size, 1);
		m_FTFont = ftgl::texture_font_new_from_file(m_FTAtlas, m_Resolution, m_Filename.c_str());

		m_Texture = Texture2D::Create(size, size, true);
	}

	ftgl::texture_glyph_t* Font::getGlyph(const char* c) {
		ftgl::texture_glyph_t* glyph = texture_font_get_glyph(m_FTFont, c);
		
		if (!m_CharsinAtlas[c]) {
			m_CharsinAtlas[c] = true;
			m_Texture->SetData(m_FTAtlas->data, m_AtlasSize);
		}

		return glyph;
	}
	
	float Font::getGlyphKerning(const ftgl::texture_glyph_t* glyph, const char* c) const {
		return texture_glyph_get_kerning(glyph, c);
	}

	const uint32_t Font::getID() const {
		return m_FTAtlas->id;
	}
}