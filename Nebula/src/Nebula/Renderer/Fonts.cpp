#include "nbpch.h"
#include "Fonts.h"

#define GL_WITH_GLAD
#define NOT_USING_FT_GL_NAMESPACE
#include <freetype-gl.h>

namespace Nebula {
	float FontGlyph::offset_x() { return m_TextureGlyph->offset_x; }
	float FontGlyph::offset_y() { return m_TextureGlyph->offset_y; }

	float FontGlyph::width()  { return m_TextureGlyph->width; }
	float FontGlyph::height() { return m_TextureGlyph->height; }

	float FontGlyph::s0() { return m_TextureGlyph->s0; }
	float FontGlyph::s1() { return m_TextureGlyph->s1; }
	float FontGlyph::t0() { return m_TextureGlyph->t0; }
	float FontGlyph::t1() { return m_TextureGlyph->t1; }

	float FontGlyph::advance_x() { return m_TextureGlyph->advance_x; }

	float FontGlyph::GetKerning(std::string_view c) const {
		return texture_glyph_get_kerning(m_TextureGlyph, c.data());
	}

	Font::Font(std::string name, std::string filename, float resolution) 
		: m_Name(name), m_Filename(filename), m_Resolution(resolution), m_Scale(vec2(m_Resolution))
	{
		RecreateAtlas();
	}

	Font::~Font() {
		ftgl::texture_atlas_delete(m_FTAtlas);
		ftgl::texture_font_delete(m_FTFont);
	}

	void Font::RecreateAtlas() {
		if (m_FTAtlas)
		{
			ftgl::texture_atlas_delete(m_FTAtlas);
			m_FTAtlas = nullptr;
		}

		if (m_FTFont) 
		{
			ftgl::texture_font_delete(m_FTFont);
			m_FTFont = nullptr;
		}

		const size_t size = (const size_t)m_Resolution * 16.0f;
		m_AtlasSize = size * size * 1.0f;

		m_FTAtlas = ftgl::texture_atlas_new(size, size, 1);
		m_FTFont = ftgl::texture_font_new_from_file(m_FTAtlas, m_Resolution, m_Filename.c_str());

		m_Texture = Texture2D::Create((const uint32_t)size, (const uint32_t)size, true);

		m_CharsinAtlas.clear();
	}

	FontGlyph Font::GetGlyph(const char* c) {
		FontGlyph glyph = texture_font_get_glyph(m_FTFont, c);
		
		if (!m_CharsinAtlas[c]) {
			m_CharsinAtlas[c] = true;
			m_Texture->SetData(m_FTAtlas->data, m_AtlasSize);
		}

		return glyph;
	}

	const uint32_t Font::GetID() const {
		return m_FTAtlas->id;
	}

	Array<Font*> FontManager::m_Fonts = {};

	void FontManager::Add(Font* font) {
		m_Fonts.push_back(font);
	}

	Font* FontManager::Get() {
		return m_Fonts[0];
	}

	Font* FontManager::Get(const std::string& name) {
		for (Font* font : m_Fonts) {
			if (font->GetName() == name)
				return font;
		}

		return nullptr;
	}

	Font* FontManager::Get(const std::string& name, uint32_t size) {
		for (Font* font : m_Fonts) {
			if (font->GetSize() == size && font->GetName() == name)
				return font;
		}

		return nullptr;
	}

	void FontManager::Clean() {
		for (uint32_t i = 0; i < m_Fonts.size(); i++)
			delete m_Fonts[i];
	}
}