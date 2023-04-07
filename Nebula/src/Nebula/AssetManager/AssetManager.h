#pragma once

#include "Nebula/Core/API.h"
#include "Nebula/Utils/Arrays.h"
#include "Asset.h"

#include <unordered_map>

namespace Nebula
{
	class AssetManager
	{
	public:
		AssetManager() = default;

		AssetHandle ImportAsset(const std::filesystem::path& path);
		AssetHandle ImportFont(const std::string& name, const std::filesystem::path& path);
		AssetHandle GetHandleFromPath(const std::filesystem::path& path);

		Ref<Asset> GetAsset(AssetHandle handle, bool load = true);
		AssetType GetAssetType(AssetHandle handle);
		
		template <typename T>
		Ref<T> GetAssetData(AssetHandle handle)
		{
			Ref<Asset> asset = GetAsset(handle);
			if (!asset)
				return nullptr;

			return asset->GetData<T>();
		}

		Array<AssetHandle> GetAllAssetsWithType(AssetType type);
	private:
		bool LoadAsset(Ref<Asset> asset);
		static void OnAssetChange(const std::string& path, const filewatch::Event change_type);
	private:
		std::unordered_map<AssetHandle, Ref<Asset>> m_Assets;
	};
}