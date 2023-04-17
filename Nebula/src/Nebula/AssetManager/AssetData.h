#pragma once

#include "Nebula/Core/UUID.h"

#include "Nebula/Renderer/Texture.h"
#include "Nebula/Renderer/Fonts.h"

namespace Nebula
{
	typedef UUID AssetHandle;

	struct AssetData
	{
		bool IsLoaded = false;
	};

	struct TextureAsset : AssetData
	{
		TextureAsset() = default;

		TextureAsset(const std::filesystem::path& path)
		{
			Texture = Texture2D::Create(path.string());
			IsLoaded = Texture->IsLoaded();
		}

		Ref<Texture2D> Texture;
	};

	struct FontAsset : AssetData
	{
		FontAsset() = default;

		FontAsset(const std::filesystem::path& path)
			: FontAsset(path.string(), path)
		{
		}

		FontAsset(const std::string& name, const std::filesystem::path& path)
		{
			Data = CreateRef<Font>(name, path);
			IsLoaded = Data->GetAtlasTexture() != nullptr;
		}

		Ref<Font> Data;
		bool Bold = false;
		bool Italic = false;
	};

	struct FontFamilyAsset : AssetData
	{
		FontFamilyAsset() = default;
		
		std::string Name = "Unknown";

		AssetHandle Regular = NULL;
		AssetHandle Bold = NULL;
		AssetHandle BoldItalic = NULL;
		AssetHandle Italic = NULL;
	};
}