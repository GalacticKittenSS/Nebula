#pragma once

#include "Texture.h"
#include "Nebula/Utils/Arrays.h"

#include <map>

extern "C" {
	typedef struct FT_LibraryRec_* FT_Library;
	typedef struct FT_FaceRec_* FT_Face;
}

namespace Nebula {
	class FontGlyph 
	{
	public:
		uint32_t character;
		float offset_x, offset_y;
		float width, height;
		float s0, s1, t0, t1;
		float advance_x;
	private:
		std::unordered_map<uint32_t, float> m_KerningValues;
		friend class Font;
	};

	struct FontGlyphPoint;
	struct FontGlyphBox;

	class Font 
	{
	public:
		Font(std::string name, std::string filename, uint32_t resolution = 32);
		~Font();

		inline const Ref<Texture2D> GetTexture() const { return m_Texture; }

		inline const std::string& GetName() const { return m_Name; }
		inline const std::string& GetFilename() const { return m_Filename; }
		
		inline const float GetResolution() const { return m_Resolution; }
		inline void SetResolution(float resolution) { m_Resolution = resolution; CreateAtlas(); }
		
		FontGlyph GetGlyph(const char* c);
		
		// Freetype only supports specific (older) fonts
		// Most cases will return 0.0f
		float GetGlyphKerning(FontGlyph glyph, const char* previous) const;
	private:
		void CreateAtlas();
		void CreateGlyph(uint32_t character);
		FontGlyphPoint GetAtlasRegion(uint32_t width, uint32_t height);
		void SetAtlasRegion(unsigned char* data, const size_t& x, const size_t& y, const size_t& width, const size_t& height);
	private:
		std::string m_Name, m_Filename;
		uint32_t m_AtlasSize, m_Resolution = 32;
		
		Ref<Texture2D> m_Texture = nullptr;
		unsigned char* m_AtlasData = nullptr;

		std::unordered_map<uint32_t, FontGlyph*> m_Glyphs;
		std::map<FontGlyphPoint, uint32_t> m_GlyphPoints;
		Array<FontGlyphBox> m_GlyphBoxes;

		FT_Face m_Face = nullptr;
		friend class FontGlyph;
	};

	struct FontFamily
	{
		std::string Name = "Unknown";

		Ref<Font> Regular = nullptr;
		Ref<Font> Bold = nullptr;
		Ref<Font> BoldItalic = nullptr;
		Ref<Font> Italic = nullptr;

		// Assumes you have your font layout as:
		// directory/name/type
		FontFamily() = default;
		FontFamily(std::string directory, std::string name);

		Ref<Font> Create(std::filesystem::path path);
	};

	class FontManager
	{
	public:
		static void Init();
		static void Shutdown();

		static void Add(const Ref<Font>& font);
		static void Add(const FontFamily& family);

		static void Clean();

		static void SetFontResolution(float resolution);
		static float GetFontResolution() { return m_FontResolution; }
		
		static Array<Ref<Font>> GetFonts() { return m_Fonts; }
		static Ref<Font> GetFont(const std::string& name);
		static Ref<Font> GetFont(const std::string& name, uint32_t resolution);
		
		static const Array<FontFamily>& GetFamilies() { return m_FontFamilies; };
		static FontFamily GetFamily(const std::string& name);
	private:
		static Array<Ref<Font>> m_Fonts;
		static Array<FontFamily> m_FontFamilies;

		static uint32_t m_FontResolution;

		static FT_Library m_Freetype;
		friend class Font;
	};
}