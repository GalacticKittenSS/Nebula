#pragma once

#include "Nebula/Project/Project.h"

namespace Nebula
{
	class AssetManager
	{
	public:
		static AssetHandle ImportAsset(const std::filesystem::path& path)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->ImportAsset(path);
		}

		static AssetHandle GetHandleFromPath(const std::filesystem::path& path)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->GetHandleFromPath(path);
		}

		static Ref<Asset> GetAsset(AssetHandle handle, bool load = true)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->GetAsset(handle, load);
		}

		static AssetType GetAssetType(AssetHandle handle)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->GetAssetType(handle);
		}

		template <typename T>
		static Ref<T> GetAssetData(AssetHandle handle)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->GetAssetData<T>(handle);
		}

		static Array<AssetHandle> GetAllAssetsWithType(AssetType type)
		{
			NB_ASSERT(Project::GetAssetManager());
			return Project::GetAssetManager()->GetAllAssetsWithType(type);
		}
	};
}