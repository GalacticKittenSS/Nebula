#pragma once

#include "Nebula/Core/API.h"
#include "Nebula/Utils/Arrays.h"
#include "Asset.h"

#include <unordered_map>

namespace Nebula
{
	class AssetManagerBase
	{
	public:
		AssetManagerBase() = default;

		AssetHandle ImportAsset(const std::filesystem::path& path);
		void ImportAsset(AssetHandle handle, const std::filesystem::path& path, const std::filesystem::path& relativePath);
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

		Array<AssetHandle> GetAllAssetsWithType(AssetType type, bool global = false);
		void GetAllAssetsWithType(Array<AssetHandle>& handlesArray, AssetType type, bool global = false);
		const std::unordered_map<AssetHandle, Ref<Asset>>& GetAssets() const { return m_Assets; }

		static void ImportFontFamily(const std::filesystem::path& directory, std::string name);
	private:
		bool LoadAsset(Ref<Asset> asset);
		static AssetHandle ImportFont(const std::string& name, const std::filesystem::path path, bool bold = false, bool italic = false);
		
		static void OnAssetChange(const std::string& path, const filewatch::Event change_type); 
		
		Ref<Asset> FindAsset(AssetHandle handle);
		static Ref<Asset> FindGlobalAsset(AssetHandle handle);
	private:
		std::unordered_map<AssetHandle, Ref<Asset>> m_Assets;
		static std::unordered_map<AssetHandle, Ref<Asset>> s_GlobalAssets;
		static uint16_t s_NextGlobalIndex;
	};
}