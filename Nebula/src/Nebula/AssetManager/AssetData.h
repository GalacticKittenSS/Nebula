#pragma once

#include "Nebula/Renderer/Texture.h"
#include "Nebula/Renderer/Fonts.h"

namespace Nebula
{
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
			FontManager::Add(Data);

			IsLoaded = Data->GetAtlasTexture() != nullptr;
		}

		Ref<Font> Data;
	};
}