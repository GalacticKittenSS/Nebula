#pragma once

#include "Texture.h"
#include "Nebula/Utils/Arrays.h"

#include <map>

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

	class Font 
	{
	public:
		Font(const std::filesystem::path& filename);
		~Font();

		inline const Ref<Texture2D> GetTexture() const { return nullptr; }
		inline const std::filesystem::path& GetFilename() const { return m_Filename; }
		
		FontGlyph GetGlyph(const char* c);
		float GetGlyphKerning(FontGlyph glyph, const char* previous) const;
	private:
		std::filesystem::path m_Filename;
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
		static void Add(const Ref<Font>& font);
		static void Add(const FontFamily& family);

		static void Clean();

		static void SetFontResolution(float resolution);
		static float GetFontResolution() { return m_FontResolution; }
		
		static const Array<Ref<Font>>& GetFonts() { return m_Fonts; }
		static Ref<Font> GetFont(const std::filesystem::path& path);
		
		static const Array<FontFamily>& GetFamilies() { return m_FontFamilies; };
		static FontFamily GetFamily(const std::string& name);
	private:
		static Array<Ref<Font>> m_Fonts;
		static Array<FontFamily> m_FontFamilies;

		static uint32_t m_FontResolution;

		friend class Font;
	};
}