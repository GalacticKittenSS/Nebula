#pragma once

#include "Nebula/Core/API.h"
#include "Nebula/Utils/Arrays.h"
#include "Nebula/AssetManager/Asset.h"
#include "Nebula/Renderer/Texture.h"

namespace Nebula {
	struct MSDFData;
	
	class Font : public Asset
	{
	public:
		Font(const std::filesystem::path& filename, std::filesystem::path cachePath = "");
		~Font();

		const MSDFData* GetMSDFData() const { return m_Data.get(); }
		inline const Ref<Texture2D> GetAtlasTexture() const { return m_AtlasTexture; }
		inline const std::filesystem::path& GetFilename() const { return m_Filename; }
		
		static Ref<Font> GetDefault();

		static AssetType GetStaticType() { return AssetType::Font; }
		virtual AssetType GetType() const { return GetStaticType(); }
	private:
		std::filesystem::path m_Filename;
		Scope<MSDFData> m_Data;
		Ref<Texture2D> m_AtlasTexture;
	};

	class FontFamily : public Asset
	{
	public:
		AssetHandle Regular = NULL;
		AssetHandle Bold = NULL;
		AssetHandle BoldItalic = NULL;
		AssetHandle Italic = NULL;

		static AssetType GetStaticType() { return AssetType::FontFamily; }
		virtual AssetType GetType() const { return GetStaticType(); }
	};
}