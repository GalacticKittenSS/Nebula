#include "nbpch.h"
#include "Fonts.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Nebula {
	struct FontGlyphPoint : public vec<2, uint32_t>
	{
		FontGlyphPoint() : vec<2, uint32_t>() {}
		FontGlyphPoint(uint32_t _x) : vec<2, uint32_t>(_x) {}
		FontGlyphPoint(uint32_t _x, uint32_t _y) : vec<2, uint32_t>(_x, _y) {}
		FontGlyphPoint(vec<2, uint32_t> vec) {
			x = vec.x;
			y = vec.y;
		}

		bool IsZero() const { return x == 0 && y == 0; }

		bool operator>(const vec<2, uint32_t>& other) const { return other.y > y || other.x > other.y; }
		bool operator<(const FontGlyphPoint& other) const
		{
			if (other.y == y)
				return other.x > x;

			return other.y > y;
		}
	};

	struct FontGlyphBox
	{
		FontGlyphPoint Min;
		FontGlyphPoint Max;

		FontGlyphBox() = default;
		FontGlyphBox(FontGlyphPoint min, FontGlyphPoint max) : Min(min), Max(max) {}

		bool Overlap(FontGlyphBox other)
		{
			return Max.x > other.Min.x && Min.x < other.Max.x
				&& Max.y > other.Min.y && Min.y < other.Max.y;
		}
	};

	Font::Font(std::string name, std::string filename, uint32_t resolution) 
		: m_Name(name), m_Filename(filename), m_Resolution(resolution)
	{

		FT_New_Face(FontManager::m_Freetype, m_Filename.c_str(), 0, &m_Face);
		CreateAtlas();
	}

	Font::~Font() 
	{
		for (auto& [i, glyph] : m_Glyphs)
			delete glyph;
		
		if (m_Face)
			FT_Done_Face(m_Face);
	}

	void Font::CreateAtlas()
	{
		m_Glyphs.clear();
		m_GlyphBoxes.clear();
		m_GlyphPoints.clear();
		delete m_AtlasData;

		m_AtlasSize = m_Resolution * 8;
		m_Texture = Texture2D::Create(m_AtlasSize, m_AtlasSize, true);
		m_AtlasData = (unsigned char*)calloc(m_AtlasSize * m_AtlasSize, sizeof(unsigned char));
		
		FT_Set_Pixel_Sizes(m_Face, 0, m_Resolution);
	}

	FontGlyphPoint Font::GetAtlasRegion(uint32_t width, uint32_t height)
	{
		FontGlyphPoint size = { width, height };

		for (auto [point, referenceCount] : m_GlyphPoints)
		{
			bool alreadyUsed = referenceCount == 3 || referenceCount >= 5;
			bool wouldExceedBounds = point + size > FontGlyphPoint(m_AtlasSize);

			if (point.IsZero() || alreadyUsed || wouldExceedBounds)
				continue;

			// Check for overlap
			FontGlyphBox newShape(point, point + size);

			bool overlaps = false;
			for (FontGlyphBox& box : m_GlyphBoxes)
			{
				if (box.Overlap(newShape))
				{
					overlaps = true;
					break;
				}
			}

			if (overlaps)
				continue;

			return point;
		}

		return {};
	}

	void Font::SetAtlasRegion(unsigned char* data, const size_t& x, const size_t& y, const size_t& width, const size_t& height)
	{
		size_t charsize = sizeof(char);

		for (int i = 0; i < height; ++i)
		{
			memcpy(m_AtlasData + ((y + i) * m_AtlasSize + x) * charsize,
				data + (i * width) * charsize, width * charsize);
		}

		m_Texture->SetData(m_AtlasData, m_AtlasSize * m_AtlasSize);
	}

	void Font::CreateGlyph(uint32_t character)
	{
		FT_Load_Char(m_Face, character, FT_LOAD_RENDER);

		FT_GlyphSlot slot = m_Face->glyph;
		FT_Bitmap ft_bitmap = slot->bitmap;

		struct {
			int8_t left;
			int8_t top;
			int8_t right;
			int8_t bottom;
		} padding = { 2, 2, 2, 2 };

		size_t src_w = ft_bitmap.width;
		size_t src_h = ft_bitmap.rows;

		size_t width  = src_w + padding.left + padding.right;
		size_t height = src_h + padding.top + padding.bottom;
		
		FontGlyphPoint region = GetAtlasRegion(width, height);

		FontGlyph* glyph = new FontGlyph();
		m_Glyphs[character] = glyph;

		glyph->character = character;
		
		glyph->width = width;
		glyph->height = height;
		
		glyph->offset_x = slot->bitmap_left;
		glyph->offset_y = slot->bitmap_top;
		glyph->advance_x = slot->advance.x / 64;
		
		uint32_t x0 = region.x;
		uint32_t y0 = region.y;
		uint32_t x1 = region.x + width;
		uint32_t y1 = region.y + height;

		glyph->s0 = (float)x0 / m_AtlasSize;
		glyph->t0 = (float)y0 / m_AtlasSize;
		glyph->s1 = (float)x1 / m_AtlasSize;
		glyph->t1 = (float)y1 / m_AtlasSize;

		m_GlyphBoxes.push_back({ { x0, y0 }, { x1, y1 } });

		FontGlyphPoint points[4] = {
			{ x0, y0 },
			{ x1, y0 },
			{ x0, y1 },
			{ x1, y1 }
		};

		for (FontGlyphPoint& point : points)
		{
			uint32_t& referenceCount = m_GlyphPoints[point];
			referenceCount++;

			// Each Point will be referenced no less than 2
			referenceCount = std::max<uint32_t>(referenceCount, 2);
		}

		// Add Padding
		unsigned char* buffer = (unsigned char*)calloc(width * height, sizeof(unsigned char));

		unsigned char* dst_ptr = buffer + (padding.top * width + padding.left);
		unsigned char* src_ptr = ft_bitmap.buffer;

		for(int i = 0; i < src_h; i++) 
		{
            memcpy(dst_ptr, src_ptr, ft_bitmap.width);
            dst_ptr += width;
            src_ptr += ft_bitmap.pitch;
        }

		SetAtlasRegion(buffer, region.x, region.y, width, height);
	}

	FontGlyph Font::GetGlyph(const char* c) 
	{
		uint32_t character = c[0];

		auto it = m_Glyphs.find(character);
		if (it == m_Glyphs.end())
			CreateGlyph(character);
		
		return *m_Glyphs.at(character);
	}

	float Font::GetGlyphKerning(FontGlyph glyph, const char* previous) const
	{
		uint32_t character = previous[0];

		auto it = glyph.m_KerningValues.find(character);
		if (it == glyph.m_KerningValues.end())
		{
			FT_Vector ft_kerning;
			FT_Get_Kerning(m_Face, character, glyph.character, FT_KERNING_UNFITTED, &ft_kerning);

			glyph.m_KerningValues[character] = ft_kerning.x;
			return ft_kerning.x;
		}

		return it->second;
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

		return CreateRef<Font>(path.string(), path.string(), FontManager::GetFontResolution());
	}

	Array<Ref<Font>> FontManager::m_Fonts = {};
	Array<FontFamily> FontManager::m_FontFamilies = {};

	uint32_t FontManager::m_FontResolution = 86.0f;

	FT_Library FontManager::m_Freetype;

	void FontManager::Init()
	{
		FT_Init_FreeType(&m_Freetype);
	}

	void FontManager::Shutdown()
	{
		FT_Done_FreeType(m_Freetype);
	}

	void FontManager::Add(const Ref<Font>& font)
	{ 
		m_Fonts.push_back(font); 
	}

	void FontManager::Add(const FontFamily& family)
	{ 
		m_FontFamilies.push_back(family); 
	}

	Ref<Font> FontManager::GetFont(const std::string& name) 
	{
		for (Ref<Font> font : m_Fonts) 
		{
			if (font->GetName() == name)
				return font;
		}

		return nullptr;
	}

	Ref<Font> FontManager::GetFont(const std::string& name, uint32_t resolution) 
	{
		for (Ref<Font> font : m_Fonts) 
		{
			if (font->GetResolution() == resolution && font->GetName() == name)
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