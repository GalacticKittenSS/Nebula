#pragma once

#include "Nebula/Core/API.h"
#include "Nebula/Utils/Arrays.h"
#include "Asset.h"

#include <unordered_map>

namespace Nebula
{
	using AssetMap = std::unordered_map<AssetHandle, Ref<Asset>>;
	using AssetRegistry = std::unordered_map<AssetHandle, AssetMetadata>;

	class AssetManagerBase
	{
	public:
		AssetManagerBase() = default;

		AssetHandle CreateAsset(const std::filesystem::path& path);
		bool CreateAsset(AssetMetadata& metadata);
		bool CreateAsset(AssetHandle handle, const std::filesystem::path& path, const std::filesystem::path& relativePath);
		
		static bool CreateGlobalAsset(AssetMetadata& metadata);
		static AssetHandle CreateGlobalAsset(const std::filesystem::path& path, const std::filesystem::path& relativePath);
		
		Ref<Asset> GetAsset(AssetHandle handle, bool load = true);
		const AssetMetadata& GetAssetMetadata(AssetHandle handle) const;
		const AssetMetadata& GetAssetMetadata(const std::filesystem::path& path) const;
		AssetHandle GetHandleFromPath(const std::filesystem::path& path);
		
		bool IsHandleValid(AssetHandle handle);
		bool IsAssetLoaded(AssetHandle handle);

		Array<AssetHandle> GetAllAssetsWithType(AssetType type, bool global = true);
		void GetAllAssetsWithType(Array<AssetHandle>& handlesArray, AssetType type, bool global = true);
		const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }

		void SerializeRegistry(const std::filesystem::path& path);
		bool DeserializeRegistry(const std::filesystem::path& path);
	private:
		//static void OnAssetChange(const std::string& path, const filewatch::Event change_type); 
		
		Ref<Asset> FindAsset(AssetHandle handle);
		static Ref<Asset> FindGlobalAsset(AssetHandle handle);
	private:
		AssetRegistry m_AssetRegistry;
		AssetMap m_Assets;

		static AssetRegistry s_GlobalRegistry;
		static uint16_t s_GlobalIndex;
	};
}