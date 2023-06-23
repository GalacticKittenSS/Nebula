#pragma once

#include "Asset.h"

namespace Nebula
{
	class AssetImporter
	{
	public:
		static Ref<Asset> ImportAsset(AssetHandle handle, const AssetMetadata& metadata);
	};
}