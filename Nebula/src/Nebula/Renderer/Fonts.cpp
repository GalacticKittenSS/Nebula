#include "nbpch.h"
#include "Fonts.h"

#undef INFINITE
#include "msdf-atlas-gen.h"

namespace Nebula 
{
	Font::Font(const std::filesystem::path& filename)
	{
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (!ft)
			return;

		std::string fileString = filename.string();
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, fileString.c_str());
			
		if (font) 
		{
			msdfgen::Shape shape;
			if (msdfgen::loadGlyph(shape, font, 'A'))
			{
				shape.normalize();
				msdfgen::edgeColoringSimple(shape, 3.0);
				msdfgen::Bitmap<float, 3> msdf(32, 32);
				msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
				msdfgen::savePng(msdf, "output.png");
			}

			msdfgen::destroyFont(font);
		}

		msdfgen::deinitializeFreetype(ft);
	}

	Font::~Font() 
	{
	}

	FontGlyph Font::GetGlyph(const char* c) 
	{
		return {};
	}

	float Font::GetGlyphKerning(FontGlyph glyph, const char* previous) const
	{
		return 0.0f;
	}

	FontFamily::FontFamily(std::string directory, std::string name)
		: Name(name)
	{
		std::filesystem::path path = directory + "/" + name;
		Regular = Create(path / "Regular.ttf");
		Bold = Create(path / "Bold.ttf");
		BoldItalic = Create(path / "BoldItalic.ttf");
		Italic = Create(path / "Italic.ttf");
	}

	Ref<Font> FontFamily::Create(const std::filesystem::path path)
	{
		if (!std::filesystem::exists(path))
			return nullptr;

		return CreateRef<Font>(path.string());
	}

	Array<Ref<Font>> FontManager::m_Fonts = {};
	Array<FontFamily> FontManager::m_FontFamilies = {};

	uint32_t FontManager::m_FontResolution = 86.0f;

	void FontManager::Add(const Ref<Font>& font)
	{ 
		m_Fonts.push_back(font); 
	}

	void FontManager::Add(const FontFamily& family)
	{ 
		m_FontFamilies.push_back(family);

		if (family.Bold)
			m_Fonts.push_back(family.Bold);

		if (family.BoldItalic)
			m_Fonts.push_back(family.BoldItalic);
		
		if (family.Italic)
			m_Fonts.push_back(family.Italic);
		
		if (family.Regular)
			m_Fonts.push_back(family.Regular);
	}

	void FontManager::SetFontResolution(float resolution)
	{
		m_FontResolution = resolution;
	}

	Ref<Font> FontManager::GetFont(const std::filesystem::path& path)
	{
		for (Ref<Font> font : m_Fonts) 
		{
			if (font->GetFilename() == path)
				return font;
		}

		return nullptr;
	}

	FontFamily FontManager::GetFamily(const std::string& name)
	{
		for (FontFamily font : m_FontFamilies)
		{
			if (font.Name == name)
				return font;
		}

		return {};
	}

	void FontManager::Clean() 
	{
		m_Fonts.clear();
		m_FontFamilies.clear();
	}
}