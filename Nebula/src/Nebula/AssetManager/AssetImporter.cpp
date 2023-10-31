#include "nbpch.h"
#include "AssetImporter.h"

#include "TextureImporter.h"
#include "FontImporter.h"
#include "MaterialImporter.h"

#include "AssetManager.h"
#include "Nebula/Core/Application.h"

#include <map>

namespace Nebula
{
	using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;
	static std::map<AssetType, AssetImportFunction> s_AssetImportFuncs = {
		{ AssetType::Font, FontImporter::ImportFont },
		{ AssetType::FontFamily, FontImporter::ImportFontFamily },
		{ AssetType::Texture, TextureImporter::ImportTexture2D },
		{ AssetType::Material, MaterialImporter::ImportMaterial },
	};

	Ref<Asset> AssetImporter::ImportAsset(AssetHandle handle, const AssetMetadata& metadata)
	{
		auto func = s_AssetImportFuncs.find(metadata.Type);
		if (func == s_AssetImportFuncs.end())
		{
			NB_ERROR("No importer func available for type: {}", metadata.Type);
			return nullptr;
		}

		Ref<Asset> asset = func->second(handle, metadata);
		if (asset)
			asset->Handle = handle;

		return asset;
	}
}