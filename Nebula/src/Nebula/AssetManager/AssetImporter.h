#pragma once

#include "Asset.h"

namespace Nebula
{
	class AssetImporter
	{
	public:
		static Ref<Asset> ImportAsset(AssetHandle handle, const AssetMetadata& metadata);
		static void OnAssetChange(const std::filesystem::path& path, const filewatch::Event change_type);
	};
}