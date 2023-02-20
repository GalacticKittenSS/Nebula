#include "nbpch.h"
#include "Fonts.h"

#undef INFINITE
#include "msdf-atlas-gen.h"
#include "FontGeometry.h"
#include "GlyphGeometry.h"

#include "MSDFData.h"

namespace Nebula 
{
	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs, 
		msdf_atlas::FontGeometry fontGeometry, uint32_t width, uint32_t height)
	{
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;

		msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
		generator.setAttributes(attributes);
		generator.setThreadCount(8);
		generator.generate(glyphs.data(), glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

		TextureSpecification spec;
		spec.Width = bitmap.width;
		spec.Height = bitmap.height;
		spec.Format = ImageFormat::RGB8;
		spec.GenerateMips = false;

		Ref<Texture2D> texture = Texture2D::Create(spec);
		texture->SetData((void*)bitmap.pixels, bitmap.width * bitmap.height * 3);
		return texture;
	}

	Font::Font(const std::filesystem::path& filename)
		: m_Data(CreateScope<MSDFData>())
	{
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		NB_ASSERT(ft);

		std::string fileString = filename.string();
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, fileString.c_str());

		if (!font)
		{
			NB_ERROR("Failed to load font: {}", fileString);
			return;
		}

		// From imgui_draw.cpp
		struct CharsetRange { uint32_t Begin, End; };
		static const CharsetRange charsetRanges[] = { { 0x0020, 0x00FF } };

		msdf_atlas::Charset charset;
		for (CharsetRange range : charsetRanges)
		{
			for (uint32_t c = range.Begin; c <= range.End; c++)
				charset.add(c);
		}

		double fontScale = 1.0;
		m_Data->FontGeometry = msdf_atlas::FontGeometry(&m_Data->Glyphs);
		int glyphsLoaded = m_Data->FontGeometry.loadCharset(font, fontScale, charset);
		NB_INFO("Loaded {} glyphs from font (out of {})", glyphsLoaded, charset.size());

		double emSize = 40.0;

		msdf_atlas::TightAtlasPacker atlasPacker;
		//atlasPacker.setDimensionsConstraint();
		atlasPacker.setPixelRange(2.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setPadding(0);
		atlasPacker.setScale(emSize);

		int remaining = atlasPacker.pack(m_Data->Glyphs.data(), (int)m_Data->Glyphs.size());
		NB_ASSERT(remaining == 0);

		int width, height;
		atlasPacker.getDimensions(width, height);
		emSize = atlasPacker.getScale();

#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define THREAD_COUNT 8

		uint64_t coloringSeed = 0;
		bool expensiveColoring = false;
		if (expensiveColoring) 
		{
			msdf_atlas::Workload([&glyphs = m_Data->Glyphs, &coloringSeed](int i, int threadNo) -> bool {
				unsigned long long glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
				glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
				return true;
			}, m_Data->Glyphs.size()).finish(THREAD_COUNT);
		}
		else 
		{
			unsigned long long glyphSeed = coloringSeed;
			for (msdf_atlas::GlyphGeometry& glyph : m_Data->Glyphs) 
			{
				glyphSeed *= LCG_MULTIPLIER;
				glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			}
		}

		m_AtlasTexture = CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>("Test", (float)emSize, m_Data->Glyphs, m_Data->FontGeometry, width, height);
		
#if 0
		msdfgen::Shape shape;
		if (msdfgen::loadGlyph(shape, font, 'A'))
		{
			shape.normalize();
			msdfgen::edgeColoringSimple(shape, 3.0);
			msdfgen::Bitmap<float, 3> msdf(32, 32);
			msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
			msdfgen::savePng(msdf, "output.png");
		}
#endif

		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);
	}

	Font::~Font() 
	{
	}

	Ref<Font> Font::GetDefault()
	{
		static Ref<Font> DefaultFont;
		if (!DefaultFont)
			DefaultFont = CreateRef<Font>("Resources/font/OpenSans/Regular.ttf");

		return DefaultFont;
	}

	FontFamily::FontFamily(std::string directory, std::string name)
		: Name(name)
	{
		std::filesystem::path path = directory + "/" + name;
		Regular = Create(path / "Regular.ttf");
		//Bold = Create(path / "Bold.ttf");
		//BoldItalic = Create(path / "BoldItalic.ttf");
		//Italic = Create(path / "Italic.ttf");
	}

	Ref<Font> FontFamily::Create(const std::filesystem::path path)
	{
		if (!std::filesystem::exists(path))
			return nullptr;

		return CreateRef<Font>(path.string());
	}

	Array<Ref<Font>> FontManager::m_Fonts = {};
	Array<FontFamily> FontManager::m_FontFamilies = {};

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