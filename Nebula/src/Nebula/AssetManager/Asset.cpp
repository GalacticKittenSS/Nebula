#include "nbpch.h"
#include "AssetData.h"
#include "Asset.h"

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
}