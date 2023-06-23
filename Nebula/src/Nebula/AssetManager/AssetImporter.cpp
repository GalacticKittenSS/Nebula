#include "nbpch.h"
#include "AssetImporter.h"

#include "TextureImporter.h"
#include "FontImporter.h"

#include <map>

namespace Nebula
{
	using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;
	static std::map<AssetType, AssetImportFunction> s_AssetImportFuncs = {
		{ AssetType::Font, FontImporter::ImportFont },
		{ AssetType::FontFamily, FontImporter::ImportFontFamily },
		{ AssetType::Texture, TextureImporter::ImportTexture2D },
	};

	Ref<Asset> AssetImporter::ImportAsset(AssetHandle handle, const AssetMetadata& metadata)
	{
		auto func = s_AssetImportFuncs.find(metadata.Type);
		if (func == s_AssetImportFuncs.end())
		{
			NB_ERROR("No importer func available for type: {}", metadata.Type);
			return nullptr;
		}

		return func->second(handle, metadata);
	}
}