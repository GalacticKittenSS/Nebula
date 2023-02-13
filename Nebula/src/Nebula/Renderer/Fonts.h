#pragma once

#include "Nebula/Core/API.h"
#include "Nebula/Utils/Arrays.h"
#include "Nebula/Renderer/Texture.h"

namespace Nebula {
	struct MSDFData;
	
	class Font 
	{
	public:
		Font(const std::filesystem::path& filename);
		~Font();

		const MSDFData* GetMSDFData() const { return m_Data.get(); }
		inline const Ref<Texture2D> GetAtlasTexture() const { return m_AtlasTexture; }
		inline const std::filesystem::path& GetFilename() const { return m_Filename; }
	private:
		std::filesystem::path m_Filename;
		Scope<MSDFData> m_Data;
		Ref<Texture2D> m_AtlasTexture;
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