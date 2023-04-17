#include "nbpch.h"
#include "Asset.h"

#include "AssetData.h"
#include "AssetManager.h"

namespace Nebula
{
	template<typename T>
	Ref<T> Asset::GetData() 
	{ 
		return nullptr; 
	}

	template<>
	Ref<Texture2D> Asset::GetData() 
	{
		if (!Data || !Data->IsLoaded
			|| Type != AssetType::Texture)
			return nullptr;

		TextureAsset* data = (TextureAsset*)Data;
		return data->Texture;
	}

	template <>
	Ref<Font> Asset::GetData() 
	{
		if (!Data || !Data->IsLoaded
			|| Type != AssetType::Font)
			return nullptr;
		
		FontAsset* data = (FontAsset*)Data;
		return data->Data;
	}

	template<typename T>
	Ref<T> Asset::GetData(bool bold, bool italic)
	{
		return nullptr;
	}

	template <>
	Ref<Font> Asset::GetData(bool bold, bool italic)
	{
		if (!Data || !Data->IsLoaded
			|| Type != AssetType::FontFamily)
			return nullptr;

		FontFamilyAsset* data = (FontFamilyAsset*)Data;

		if (bold && italic)
			return AssetManager::GetAssetData<Font>(data->BoldItalic);
		
		if (bold)
			return AssetManager::GetAssetData<Font>(data->Bold);

		if (italic)
			return AssetManager::GetAssetData<Font>(data->Italic);

		return AssetManager::GetAssetData<Font>(data->Regular);
	}
}